#include "StepIndicatorWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>

namespace fc {

namespace {
struct StepDef { const char *label; CheckpointState activeState; CheckpointState waitState; };
constexpr StepDef kSteps[] = {
    {"Лицо",      CheckpointState::FaceMatch,      CheckpointState::GuardVerifyIdentity},
    {"Металл",    CheckpointState::MetalCheck,      CheckpointState::GuardMetal},
    {"Темп.",     CheckpointState::TempCheck,       CheckpointState::MedicTemp},
    {"Алкоголь",  CheckpointState::AlcoholCheck,    CheckpointState::MedicAlcohol},
    {"Турникет",  CheckpointState::Granted,         CheckpointState::Idle},
};

bool stateIndex(CheckpointState s, int &outIdx, bool &outWaiting) {
    for (int i = 0; i < 5; ++i) {
        if (s == kSteps[i].activeState) { outIdx = i; outWaiting = false; return true; }
        if (s == kSteps[i].waitState)   { outIdx = i; outWaiting = true;  return true; }
    }
    return false;
}
}

StepIndicatorWidget::StepIndicatorWidget(QWidget *parent) : QWidget(parent) {
    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(4, 4, 4, 4);
    lay->setSpacing(0);

    for (int i = 0; i < Count; ++i) {
        if (i > 0) {
            auto *sep = new QLabel("›", this);
            sep->setStyleSheet("color:#666;font-size:16px;padding:0 4px");
            lay->addWidget(sep);
        }
        auto *lbl = new QLabel(kSteps[i].label, this);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setFixedHeight(32);
        lbl->setMinimumWidth(80);
        lbl->setStyleSheet("border-radius:4px;background:#333;color:#888;padding:0 8px");
        lay->addWidget(lbl);
        labels_.push_back(lbl);
    }
    setStyleSheet("background:#1a1a1a");
}

void StepIndicatorWidget::setState(CheckpointState state, DenyReason deny) {
    int activeIdx = -1;
    bool waiting = false;
    stateIndex(state, activeIdx, waiting);

    if (state == CheckpointState::FaceCapture || state == CheckpointState::FaceMatch
            || state == CheckpointState::GuardVerifyIdentity) {
        activeIdx = 0;
        waiting = (state == CheckpointState::GuardVerifyIdentity);
    }

    const bool isDenied = (state == CheckpointState::Denied);
    const bool isGranted = (state == CheckpointState::Granted);

    for (int i = 0; i < labels_.size(); ++i) {
        QLabel *lbl = labels_[i];
        if (state == CheckpointState::Idle) {
            lbl->setStyleSheet("border-radius:4px;background:#333;color:#888;padding:0 8px");
        } else if (i < activeIdx) {
            // пройденный шаг
            lbl->setStyleSheet("border-radius:4px;background:#1e6b1e;color:#9ef09e;padding:0 8px;font-weight:bold");
        } else if (i == activeIdx) {
            if (isDenied) {
                lbl->setStyleSheet("border-radius:4px;background:#7a1a1a;color:#f08080;padding:0 8px;font-weight:bold");
            } else if (waiting) {
                lbl->setStyleSheet("border-radius:4px;background:#6b4e1a;color:#ffd080;padding:0 8px;font-weight:bold");
            } else {
                lbl->setStyleSheet("border-radius:4px;background:#1a4a7a;color:#80c0f0;padding:0 8px;font-weight:bold");
            }
        } else if (isGranted) {
            lbl->setStyleSheet("border-radius:4px;background:#1e6b1e;color:#9ef09e;padding:0 8px;font-weight:bold");
        } else {
            lbl->setStyleSheet("border-radius:4px;background:#333;color:#555;padding:0 8px");
        }
    }
}

void StepIndicatorWidget::reset() {
    setState(CheckpointState::Idle);
}

}   // namespace fc
