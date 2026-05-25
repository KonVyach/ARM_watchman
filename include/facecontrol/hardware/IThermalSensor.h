#pragma once

#include <QObject>

namespace fc {

class IThermalSensor : public QObject {
    Q_OBJECT
public:
    explicit IThermalSensor(QObject *parent = nullptr) : QObject(parent) {}
    ~IThermalSensor() override;

    // Текущая измеренная температура (°C). Может вернуть NaN, если ещё не измерено.
    virtual double currentTemperature() const = 0;

    // Запросить разовое измерение; результат — через сигнал.
    virtual void requestReading() = 0;

signals:
    void temperatureRead(double celsius);
};

}   // namespace fc
