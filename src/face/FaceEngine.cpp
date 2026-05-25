#include "FaceEngine.h"

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/objdetect.hpp>

#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>

#include <algorithm>
#include <cmath>

namespace fc {

namespace {

cv::Mat qimageToBgr(const QImage &src) {
    QImage rgb = src.convertToFormat(QImage::Format_RGB888);
    cv::Mat m(rgb.height(), rgb.width(), CV_8UC3,
              const_cast<uchar*>(rgb.constBits()),
              static_cast<size_t>(rgb.bytesPerLine()));
    cv::Mat out;
    cv::cvtColor(m, out, cv::COLOR_RGB2BGR);
    return out.clone();
}

void l2_normalize(std::vector<float> &v) {
    double norm = 0.0;
    for (float x : v) norm += static_cast<double>(x) * x;
    norm = std::sqrt(norm);
    if (norm < 1e-9) return;
    for (float &x : v) x = static_cast<float>(x / norm);
}

// Аффинное выравнивание 112x112 по 5 ключевым точкам YuNet (точный ArcFace-шаблон)
cv::Mat alignFaceYuNet(const cv::Mat &bgr, const std::vector<cv::Point2f> &lm5) {
    static const cv::Point2f kDst[5] = {
        {38.2946f, 51.6963f}, {73.5318f, 51.5014f}, {56.0252f, 71.7366f},
        {41.5493f, 92.3655f}, {70.7299f, 92.2041f}
    };
    std::vector<cv::Point2f> dst(std::begin(kDst), std::end(kDst));
    cv::Mat M = cv::estimateAffinePartial2D(lm5, dst);
    if (M.empty()) return {};
    cv::Mat aligned;
    cv::warpAffine(bgr, aligned, M, cv::Size(112, 112), cv::INTER_LINEAR);
    return aligned;
}

// Простой кроп без выравнивания (fallback если YuNet недоступен)
cv::Mat cropFace(const cv::Mat &bgr, const cv::Rect &box) {
    int px = static_cast<int>(box.width  * 0.1f);
    int py = static_cast<int>(box.height * 0.1f);
    cv::Rect safe = cv::Rect(box.x - px, box.y - py,
                             box.width + 2*px, box.height + 2*py)
                    & cv::Rect(0, 0, bgr.cols, bgr.rows);
    if (safe.area() <= 0) return {};
    cv::Mat crop = bgr(safe), out;
    cv::resize(crop, out, cv::Size(112, 112), 0, 0, cv::INTER_LINEAR);
    return out;
}

std::string findCascade() {
    try {
        std::string f = cv::samples::findFile("haarcascade_frontalface_default.xml", false);
        if (!f.empty()) return f;
    } catch (...) {}

    std::string exeDir = QCoreApplication::applicationDirPath().toStdString();
    std::vector<std::string> cands = {
        exeDir + "/models/haarcascade_frontalface_default.xml",
        exeDir + "/haarcascade_frontalface_default.xml",
        "C:/opencv/build/etc/haarcascades/haarcascade_frontalface_default.xml",
        "/opt/homebrew/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
        "/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
    };
    for (auto &p : cands)
        if (!p.empty() && QFileInfo::exists(QString::fromStdString(p))) return p;
    return {};
}

} // namespace

// ---------------------------------------------------------------------------

struct FaceEngine::Impl {
    // --- ArcFace embedding (cv::dnn) ---
    cv::dnn::Net arcfaceNet;

    // --- Face detector: YuNet preferred, Haar fallback ---
    cv::Ptr<cv::FaceDetectorYN> yunetDetector;   // если удалось загрузить
    cv::CascadeClassifier       haarClassifier;  // fallback
    bool useYuNet = false;
    bool useHaar  = false;

    // Результат последнего detect()
    cv::Rect                  lastBox;
    std::vector<cv::Point2f>  lastLandmarks;   // 5 точек, только при YuNet
    bool                      hasLast = false;

    bool initArcface(const std::string &path, QString *err) {
        try {
            arcfaceNet = cv::dnn::readNetFromONNX(path);
            if (arcfaceNet.empty()) { if (err) *err = "ArcFace model is empty"; return false; }
            arcfaceNet.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
            arcfaceNet.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
            return true;
        } catch (const std::exception &e) { if (err) *err = e.what(); return false; }
        catch (...) { if (err) *err = "Unknown error"; return false; }
    }

    void tryInitYuNet(const std::string &path) {
        if (path.empty() || !QFileInfo::exists(QString::fromStdString(path))) return;
        try {
            auto det = cv::FaceDetectorYN::create(path, "", cv::Size(320, 320), 0.6f, 0.3f, 5000);
            if (!det.empty()) { yunetDetector = det; useYuNet = true;
                qDebug() << "FaceEngine: using YuNet detector"; }
        } catch (...) {}
    }

    void tryInitHaar() {
        std::string cascPath = findCascade();
        if (!cascPath.empty() && haarClassifier.load(cascPath)) {
            useHaar = true;
            qDebug() << "FaceEngine: using Haar cascade fallback";
        }
    }
};

FaceEngine::FaceEngine(QObject *parent) : QObject(parent), impl_(std::make_unique<Impl>()) {}
FaceEngine::~FaceEngine() = default;

bool FaceEngine::initialize(const QString &yunetModelPath,
                            const QString &arcfaceModelPath,
                            QString *errorOut) {
    if (!QFileInfo::exists(arcfaceModelPath)) {
        if (errorOut) *errorOut = tr("ArcFace model not found: %1").arg(arcfaceModelPath);
        return false;
    }

    // 1. Пробуем YuNet (лучше)
    impl_->tryInitYuNet(yunetModelPath.toStdString());

    // 2. Если YuNet не загрузился — Haar cascade (всегда есть в OpenCV)
    if (!impl_->useYuNet)
        impl_->tryInitHaar();

    // 3. ArcFace обязателен
    if (!impl_->initArcface(arcfaceModelPath.toStdString(), errorOut)) return false;

    if (impl_->useYuNet)
        qDebug() << "FaceEngine: YuNet + ArcFace (full accuracy)";
    else if (impl_->useHaar)
        qDebug() << "FaceEngine: Haar + ArcFace (reduced accuracy, no alignment)";
    else
        qDebug() << "FaceEngine: ArcFace only (no detector, uses full frame)";

    return true;
}

bool FaceEngine::isReady() const {
    return !impl_->arcfaceNet.empty();
}

std::vector<DetectedFace> FaceEngine::detect(const QImage &frame) {
    impl_->hasLast = false;
    if (frame.isNull()) return {};

    cv::Mat bgr;
    try { bgr = qimageToBgr(frame); } catch (...) { return {}; }
    if (bgr.empty()) return {};

    std::vector<DetectedFace> out;

    if (impl_->useYuNet) {
        // --- YuNet ---
        impl_->yunetDetector->setInputSize(bgr.size());
        cv::Mat faces;
        try { impl_->yunetDetector->detect(bgr, faces); } catch (...) { goto fallback; }

        int bestIdx = -1; float bestArea = 0.f;
        for (int i = 0; i < faces.rows; ++i) {
            DetectedFace f;
            f.x = (int)faces.at<float>(i,0); f.y = (int)faces.at<float>(i,1);
            f.w = (int)faces.at<float>(i,2); f.h = (int)faces.at<float>(i,3);
            f.score = faces.at<float>(i,14);
            out.push_back(f);
            float area = (float)(f.w * f.h);
            if (area > bestArea) { bestArea = area; bestIdx = i; }
        }
        if (bestIdx >= 0) {
            impl_->lastBox = cv::Rect(out[bestIdx].x, out[bestIdx].y,
                                      out[bestIdx].w, out[bestIdx].h);
            impl_->lastLandmarks.clear();
            for (int k = 0; k < 5; ++k)
                impl_->lastLandmarks.emplace_back(
                    faces.at<float>(bestIdx, 4 + 2*k),
                    faces.at<float>(bestIdx, 4 + 2*k + 1));
            impl_->hasLast = true;
        }
        return out;
    }

fallback:
    if (impl_->useHaar) {
        // --- Haar cascade ---
        cv::Mat gray;
        cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);
        std::vector<cv::Rect> faces;
        try { impl_->haarClassifier.detectMultiScale(gray, faces, 1.1, 4, 0, cv::Size(30,30)); }
        catch (...) { goto fullframe; }

        int bestIdx = -1, bestArea = 0;
        for (int i = 0; i < (int)faces.size(); ++i) {
            DetectedFace f{faces[i].x, faces[i].y, faces[i].width, faces[i].height, 1.f};
            out.push_back(f);
            if (f.w * f.h > bestArea) { bestArea = f.w * f.h; bestIdx = i; }
        }
        if (bestIdx >= 0) {
            impl_->lastBox = faces[bestIdx];
            impl_->lastLandmarks.clear();
            impl_->hasLast = true;
        }
        return out;
    }

fullframe:
    // --- Нет детектора: весь кадр ---
    DetectedFace f{0, 0, bgr.cols, bgr.rows, 0.5f};
    out.push_back(f);
    impl_->lastBox = cv::Rect(0, 0, bgr.cols, bgr.rows);
    impl_->lastLandmarks.clear();
    impl_->hasLast = true;
    return out;
}

std::vector<float> FaceEngine::embedLargestFace(const QImage &frame) {
    detect(frame);
    if (!impl_->hasLast || impl_->arcfaceNet.empty()) return {};

    cv::Mat bgr;
    try { bgr = qimageToBgr(frame); } catch (...) { return {}; }

    cv::Mat aligned;
    if (impl_->useYuNet && impl_->lastLandmarks.size() == 5) {
        aligned = alignFaceYuNet(bgr, impl_->lastLandmarks);
    }
    if (aligned.empty()) {
        aligned = cropFace(bgr, impl_->lastBox);
    }
    if (aligned.empty()) return {};

    cv::Mat blob = cv::dnn::blobFromImage(
        aligned, 1.0/128.0, cv::Size(112,112),
        cv::Scalar(127.5,127.5,127.5), true, false, CV_32F);

    try {
        impl_->arcfaceNet.setInput(blob);
        cv::Mat out = impl_->arcfaceNet.forward();
        std::vector<float> emb(out.begin<float>(), out.end<float>());
        l2_normalize(emb);
        return emb;
    } catch (const std::exception &e) {
        qWarning() << "ArcFace inference failed:" << e.what();
        return {};
    } catch (...) {
        qWarning() << "ArcFace inference: unknown error";
        return {};
    }
}

} // namespace fc
