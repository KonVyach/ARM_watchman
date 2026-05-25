#include "WebcamOpenCv.h"

#include <opencv2/opencv.hpp>

#include <QMutexLocker>

namespace fc {

namespace {
QImage matToImage(const cv::Mat &bgr) {
    cv::Mat rgb;
    cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
    return QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step),
                  QImage::Format_RGB888).copy();
}
}   // namespace

WebcamOpenCv::WebcamOpenCv(int deviceIndex, QObject *parent)
    : ICamera(parent), deviceIndex_(deviceIndex) {}

WebcamOpenCv::~WebcamOpenCv() {
    stop();
}

bool WebcamOpenCv::start() {
    if (running_.load()) return true;
    running_.store(true);

    thread_ = QThread::create([this] { run(); });
    thread_->setObjectName(QStringLiteral("Webcam-%1").arg(deviceIndex_));
    thread_->start();
    return true;
}

void WebcamOpenCv::stop() {
    if (!running_.load()) return;
    running_.store(false);
    if (thread_) {
        thread_->wait(2000);
        delete thread_;
        thread_ = nullptr;
    }
}

QImage WebcamOpenCv::lastFrame() const {
    QMutexLocker lock(&frameMutex_);
    return lastFrame_;
}

void WebcamOpenCv::run() {
    cv::VideoCapture cap(deviceIndex_);
    if (!cap.isOpened()) {
        emit errorOccurred(tr("Не удалось открыть камеру %1").arg(deviceIndex_));
        running_.store(false);
        return;
    }

    cv::Mat frame;
    while (running_.load()) {
        if (!cap.read(frame) || frame.empty()) {
            QThread::msleep(20);
            continue;
        }
        QImage img = matToImage(frame);
        {
            QMutexLocker lock(&frameMutex_);
            lastFrame_ = img;
        }
        emit frameReady(img);
        QThread::msleep(33);   // ~30 fps
    }
    cap.release();
}

}   // namespace fc
