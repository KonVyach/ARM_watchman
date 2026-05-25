#pragma once

#include "facecontrol/hardware/IBreathalyzer.h"

namespace fc {

class MockBreathalyzer : public IBreathalyzer {
    Q_OBJECT
public:
    explicit MockBreathalyzer(QObject *parent = nullptr);

    IntoxicationReading lastReading() const override { return current_; }
    void requestReading() override;

public slots:
    void setSimulatedBac(double promille);
    void setSimulatedNarcotic(double score);

private:
    IntoxicationReading current_;
};

}   // namespace fc
