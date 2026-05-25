#include "MockTurnstile.h"

namespace fc {

MockTurnstile::MockTurnstile(QObject *parent)
    : ITurnstile(parent) {}

void MockTurnstile::open() {
    if (state_ == TurnstileState::Opened) return;
    state_ = TurnstileState::Opened;
    emit stateChanged(state_);
}

void MockTurnstile::close() {
    if (state_ == TurnstileState::Closed) return;
    state_ = TurnstileState::Closed;
    emit stateChanged(state_);
}

void MockTurnstile::notifyPassed() {
    emit personPassed();
}

}   // namespace fc
