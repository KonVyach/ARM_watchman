#include "ThresholdPolicy.h"
#include "db/Database.h"

#include <algorithm>

namespace fc {

namespace { constexpr auto kKey = "face.threshold"; }

ThresholdPolicy::ThresholdPolicy(QObject *parent) : QObject(parent) {}

void ThresholdPolicy::reset() {
    setValue(kInitial);
}

void ThresholdPolicy::relax() {
    setValue(std::max(kFloor, current_ - kStep));
}

void ThresholdPolicy::setValue(double v) {
    v = std::clamp(v, kFloor, 1.0);
    if (v == current_) return;
    current_ = v;
    save();
    emit changed(current_);
}

void ThresholdPolicy::load() {
    const auto stored = Database::readSetting(kKey);
    bool ok = false;
    double v = stored.toDouble(&ok);
    current_ = ok ? std::clamp(v, kFloor, 1.0) : kInitial;
    emit changed(current_);
}

void ThresholdPolicy::save() const {
    Database::writeSetting(kKey, QString::number(current_, 'f', 4));
}

}   // namespace fc
