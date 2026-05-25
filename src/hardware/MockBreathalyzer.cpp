#include "MockBreathalyzer.h"

#include <QTimer>

namespace fc {

MockBreathalyzer::MockBreathalyzer(QObject *parent)
    : IBreathalyzer(parent) {}

void MockBreathalyzer::requestReading() {
    QTimer::singleShot(0, this, [this] { emit readingReady(current_); });
}

void MockBreathalyzer::setSimulatedBac(double promille) {
    current_.bacPromille = promille;
}

void MockBreathalyzer::setSimulatedNarcotic(double score) {
    current_.narcoticScore = score;
}

}   // namespace fc
