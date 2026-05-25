#pragma once

#include "facecontrol/core/CheckpointState.h"

#include <QWidget>
#include <QVector>

class QLabel;

namespace fc {

// Горизонтальная лента из 5 шагов: Face / Metal / Temp / Alcohol / Gate.
// Подсвечивает активный шаг и помечает пройденные.
class StepIndicatorWidget : public QWidget {
    Q_OBJECT
public:
    explicit StepIndicatorWidget(QWidget *parent = nullptr);

    void setState(CheckpointState state, DenyReason deny = DenyReason::None);
    void reset();

private:
    enum StepKind { Face = 0, Metal, Temp, Alcohol, Gate, Count };

    void paintStep(int idx, int activeIdx, bool denied);

    QVector<QLabel*> labels_;
};

}   // namespace fc
