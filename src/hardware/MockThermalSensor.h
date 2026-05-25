#pragma once

#include "facecontrol/hardware/IThermalSensor.h"

namespace fc {

class MockThermalSensor : public IThermalSensor {
    Q_OBJECT
public:
    explicit MockThermalSensor(QObject *parent = nullptr);

    double currentTemperature() const override { return current_; }
    void requestReading() override;

public slots:
    // Управление с пульта симулятора.
    void setSimulatedTemperature(double celsius);

private:
    double current_ = 36.6;
};

}   // namespace fc
