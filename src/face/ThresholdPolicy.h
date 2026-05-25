#pragma once

#include <QObject>

namespace fc {

// Адаптивный порог косинусного сходства для face match.
// Стартовое значение — 0.45 (рабочее значение для ArcFace L2-normalized).
// При каждом подтверждённом охранником false-negative .relax() снижает
// порог на step (0.02), но не ниже floor (0.30) — это защита от того, чтобы
// сеть не «развалилась» и не стала пропускать кого попало.
// Хранится в таблице settings ключом "face.threshold".
class ThresholdPolicy : public QObject {
    Q_OBJECT
public:
    static constexpr double kInitial = 0.45;
    static constexpr double kFloor   = 0.30;
    static constexpr double kStep    = 0.02;

    explicit ThresholdPolicy(QObject *parent = nullptr);

    double current() const { return current_; }
    void   reset();
    void   relax();          // сбрасывает строгость на kStep вниз (но не ниже kFloor)
    void   setValue(double v);

    void load();
    void save() const;

signals:
    void changed(double value);

private:
    double current_ = kInitial;
};

}   // namespace fc
