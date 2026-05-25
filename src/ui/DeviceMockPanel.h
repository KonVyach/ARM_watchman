#pragma once

#include <QGroupBox>

class QDoubleSpinBox;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QLabel;

namespace fc {

class MockThermalSensor;
class MockMetalDetector;
class MockBreathalyzer;
class MockTurnstile;
class ITurnstile;
class IThermalSensor;
class IMetalDetector;
class IBreathalyzer;

// Правая панель-симулятор. Позволяет задавать значения всех мок-устройств
// вручную, что даёт возможность демонстрировать любые ветки пайплайна.
class DeviceMockPanel : public QGroupBox {
    Q_OBJECT
public:
    explicit DeviceMockPanel(QWidget *parent = nullptr);

    void bindDevices(IThermalSensor *thermal,
                     IMetalDetector *metal,
                     IBreathalyzer  *breath,
                     ITurnstile     *turnstile);

private:
    void applyThreshold();

    MockThermalSensor *thermal_ = nullptr;
    MockMetalDetector *metal_   = nullptr;
    MockBreathalyzer  *breath_  = nullptr;
    MockTurnstile     *turnstile_ = nullptr;

    QDoubleSpinBox *tempSpin_ = nullptr;
    QCheckBox      *metalCheck_ = nullptr;
    QLineEdit      *metalItems_ = nullptr;
    QDoubleSpinBox *bacSpin_ = nullptr;
    QDoubleSpinBox *narcSpin_ = nullptr;
    QPushButton    *passedBtn_ = nullptr;
    QLabel         *turnstileStatus_ = nullptr;
};

}   // namespace fc
