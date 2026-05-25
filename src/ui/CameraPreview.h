#pragma once

#include <QLabel>
#include <QPointer>

namespace fc {

class ICamera;

// Простой превью-виджет одной камеры: показывает поток QImage и заголовок.
class CameraPreview : public QLabel {
    Q_OBJECT
public:
    explicit CameraPreview(const QString &title, QWidget *parent = nullptr);

    void bind(ICamera *cam);
    QImage currentFrame() const { return frame_; }

protected:
    void paintEvent(QPaintEvent *e) override;

private slots:
    void onFrame(const QImage &frame);

private:
    QString          title_;
    QImage           frame_;
    QPointer<ICamera> camera_;
};

}   // namespace fc
