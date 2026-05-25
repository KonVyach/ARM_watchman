#pragma once

#include "facecontrol/hardware/IMetalDetector.h"

namespace fc {

class MockMetalDetector : public IMetalDetector {
    Q_OBJECT
public:
    explicit MockMetalDetector(QObject *parent = nullptr);

    MetalDetectorReading lastReading() const override { return current_; }
    void requestReading() override;

public slots:
    void setSimulatedTriggered(bool triggered);
    void setSimulatedItems(const QStringList &items);

private:
    MetalDetectorReading current_;
};

}   // namespace fc
