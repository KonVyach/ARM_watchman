#include "CameraPreview.h"
#include "facecontrol/hardware/ICamera.h"

#include <QPainter>

namespace fc {

CameraPreview::CameraPreview(const QString &title, QWidget *parent)
    : QLabel(parent), title_(title) {
    setMinimumSize(320, 240);
    setStyleSheet("background:#222;color:#ddd;border:1px solid #444");
    setAlignment(Qt::AlignCenter);
    setText(tr("Нет сигнала"));
}

void CameraPreview::bind(ICamera *cam) {
    if (camera_) disconnect(camera_, nullptr, this, nullptr);
    camera_ = cam;
    if (cam) connect(cam, &ICamera::frameReady, this, &CameraPreview::onFrame);
}

void CameraPreview::onFrame(const QImage &frame) {
    frame_ = frame;
    update();
}

void CameraPreview::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), QColor(34, 34, 34));
    if (!frame_.isNull()) {
        QImage scaled = frame_.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPoint origin((width() - scaled.width()) / 2, (height() - scaled.height()) / 2);
        p.drawImage(origin, scaled);
    } else {
        p.setPen(Qt::lightGray);
        p.drawText(rect(), Qt::AlignCenter, tr("Нет сигнала"));
    }
    p.setPen(Qt::white);
    p.drawText(QRect(8, 6, width() - 16, 20), Qt::AlignLeft, title_);
}

}   // namespace fc
