#pragma once

#include <QObject>

#include "facecontrol/hardware/IBreathalyzer.h"
#include "facecontrol/hardware/ICamera.h"
#include "facecontrol/hardware/IMetalDetector.h"
#include "facecontrol/hardware/IThermalSensor.h"
#include "facecontrol/hardware/ITurnstile.h"

namespace fc {

// Владеет всеми устройствами (моки или реальные) и раздаёт их по запросу.
// Дополнительно даёт прямой доступ к моковым подклассам для пульта симулятора.
class DeviceManager : public QObject {
    Q_OBJECT
public:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager() override;

    // Регистрация устройств. Передаваемые объекты становятся child-ами менеджера.
    void setFaceCamera(ICamera *cam);
    void setThermal(IThermalSensor *dev);
    void setMetal(IMetalDetector *dev);
    void setBreath(IBreathalyzer *dev);
    void setTurnstile(ITurnstile *dev);

    ICamera *faceCamera() const { return face_; }
    IThermalSensor *thermal() const { return thermal_; }
    IMetalDetector *metal() const { return metal_; }
    IBreathalyzer *breath() const { return breath_; }
    ITurnstile *turnstile() const { return turnstile_; }

    // Фабрика «всё по умолчанию» — реальные веб-камеры (индексы 0, 1) +
    // моковые остальные устройства. Если камеры открыть не удалось, возвращается
    // объект менеджера с null-камерами; UI должен обработать.
    static DeviceManager *createDefault(QObject *parent = nullptr);

private:
    ICamera *face_ = nullptr;
    IThermalSensor *thermal_ = nullptr;
    IMetalDetector *metal_ = nullptr;
    IBreathalyzer *breath_ = nullptr;
    ITurnstile *turnstile_ = nullptr;
};

}   // namespace fc
