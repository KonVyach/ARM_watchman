#pragma once

#include "facecontrol/hardware/ICamera.h"

#include <QImage>
#include <QMutex>
#include <QThread>

#include <atomic>

namespace cv { class VideoCapture; }

namespace fc {

// Веб-камера на базе OpenCV `cv::VideoCapture`. Чтение кадров — в отдельном
// потоке, последний кадр кешируется и эмитится сигналом frameReady.
class WebcamOpenCv : public ICamera {
    Q_OBJECT
public:
    explicit WebcamOpenCv(int deviceIndex, QObject *parent = nullptr);
    ~WebcamOpenCv() override;

    bool start() override;
    void stop() override;
    bool isRunning() const override { return running_.load(); }
    QImage lastFrame() const override;

    int deviceIndex() const { return deviceIndex_; }

private:
    void run();   // тело рабочего потока

    int deviceIndex_;
    std::atomic<bool> running_{false};
    QThread *thread_ = nullptr;
    mutable QMutex frameMutex_;
    QImage lastFrame_;
};

}   // namespace fc
