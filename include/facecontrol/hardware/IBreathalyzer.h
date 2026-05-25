#pragma once

#include <QObject>

namespace fc {

struct IntoxicationReading {
    double bacPromille = 0.0;       // алкоголь, ‰ (промилле)
    double narcoticScore = 0.0;     // условная оценка наркотического опьянения [0..1]
};

class IBreathalyzer : public QObject {
    Q_OBJECT
public:
    explicit IBreathalyzer(QObject *parent = nullptr) : QObject(parent) {}
    ~IBreathalyzer() override;

    virtual IntoxicationReading lastReading() const = 0;
    virtual void requestReading() = 0;

signals:
    void readingReady(const IntoxicationReading &reading);
};

}   // namespace fc
