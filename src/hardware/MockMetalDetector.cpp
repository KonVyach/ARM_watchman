#include "MockMetalDetector.h"

#include <QTimer>

namespace fc {

MockMetalDetector::MockMetalDetector(QObject *parent)
    : IMetalDetector(parent) {}

void MockMetalDetector::requestReading() {
    QTimer::singleShot(0, this, [this] { emit readingReady(current_); });
}

void MockMetalDetector::setSimulatedTriggered(bool triggered) {
    current_.triggered = triggered;
    if (!triggered) current_.items.clear();
}

void MockMetalDetector::setSimulatedItems(const QStringList &items) {
    current_.items = items;
    current_.triggered = !items.isEmpty();
}

}   // namespace fc
