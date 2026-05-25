#pragma once

#include <QObject>
#include <QStringList>

namespace fc {

struct MetalDetectorReading {
    bool triggered = false;     // обнаружен ли металл/электроника
    QStringList items;          // список обнаруженных предметов (для UI и логов)
};

class IMetalDetector : public QObject {
    Q_OBJECT
public:
    explicit IMetalDetector(QObject *parent = nullptr) : QObject(parent) {}
    ~IMetalDetector() override;

    virtual MetalDetectorReading lastReading() const = 0;
    virtual void requestReading() = 0;

signals:
    void readingReady(const MetalDetectorReading &reading);
};

}   // namespace fc
