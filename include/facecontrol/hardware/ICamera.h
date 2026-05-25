#pragma once

#include <QImage>
#include <QObject>

namespace fc {

// ICamera поставляет поток кадров с одной физической камеры.
// Реальная реализация — WebcamOpenCv. Мок-реализация не предусмотрена —
// если камеры нет, поток просто остаётся пустым.
class ICamera : public QObject {
    Q_OBJECT
public:
    explicit ICamera(QObject *parent = nullptr) : QObject(parent) {}
    ~ICamera() override;

    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;

    // Возвращает последний кадр в формате QImage::Format_RGB888.
    // Пустой QImage если камера ещё не отдала ни одного кадра.
    virtual QImage lastFrame() const = 0;

signals:
    void frameReady(const QImage &frame);
    void errorOccurred(const QString &message);
};

}   // namespace fc
