#pragma once

#include "facecontrol/hardware/ITurnstile.h"

namespace fc {

class MockTurnstile : public ITurnstile {
    Q_OBJECT
public:
    explicit MockTurnstile(QObject *parent = nullptr);

    TurnstileState state() const override { return state_; }

    void open() override;
    void close() override;
    void notifyPassed() override;

private:
    TurnstileState state_ = TurnstileState::Closed;
};

}   // namespace fc
