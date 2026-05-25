#include "MockThermalSensor.h"

#include <QTimer>

namespace fc {

MockThermalSensor::MockThermalSensor(QObject *parent)
    : IThermalSensor(parent) {}

void MockThermalSensor::requestReading() {
    QTimer::singleShot(0, this, [this] { emit temperatureRead(current_); });
}

void MockThermalSensor::setSimulatedTemperature(double celsius) {
    current_ = celsius;
}

}   // namespace fc
