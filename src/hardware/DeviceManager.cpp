#include "DeviceManager.h"

#include "MockBreathalyzer.h"
#include "MockMetalDetector.h"
#include "MockThermalSensor.h"
#include "MockTurnstile.h"
#include "WebcamOpenCv.h"

namespace fc {

DeviceManager::DeviceManager(QObject *parent) : QObject(parent) {}
DeviceManager::~DeviceManager() = default;

void DeviceManager::setFaceCamera(ICamera *cam)   { face_ = cam;   if (cam) cam->setParent(this); }
void DeviceManager::setThermal(IThermalSensor *d) { thermal_ = d;  if (d)   d->setParent(this); }
void DeviceManager::setMetal(IMetalDetector *d)   { metal_ = d;    if (d)   d->setParent(this); }
void DeviceManager::setBreath(IBreathalyzer *d)   { breath_ = d;   if (d)   d->setParent(this); }
void DeviceManager::setTurnstile(ITurnstile *d)   { turnstile_ = d;if (d)   d->setParent(this); }

DeviceManager *DeviceManager::createDefault(QObject *parent) {
    auto *mgr = new DeviceManager(parent);
    mgr->setFaceCamera(new WebcamOpenCv(0, mgr));
    mgr->setThermal(new MockThermalSensor(mgr));
    mgr->setMetal(new MockMetalDetector(mgr));
    mgr->setBreath(new MockBreathalyzer(mgr));
    mgr->setTurnstile(new MockTurnstile(mgr));
    return mgr;
}

}   // namespace fc
