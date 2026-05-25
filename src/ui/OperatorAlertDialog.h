#pragma once

#include "facecontrol/core/CheckpointState.h"
#include "facecontrol/core/Person.h"
#include "face/FaceMatcher.h"

#include <QDialog>
#include <QVector>

class QPushButton;
class QLabel;
class QListWidget;

namespace fc {

class FaceMatcher;

// Модальный диалог, появляющийся когда пайплайн требует вмешательства
// оператора (охранник или медик). Внешний вид и набор кнопок зависит от
// состояния, в котором остановился пайплайн.
class OperatorAlertDialog : public QDialog {
    Q_OBJECT
public:
    OperatorAlertDialog(CheckpointState waitState,
                        const QString &message,
                        FaceMatcher *matcher,
                        const std::vector<float> &embedding,
                        QWidget *parent = nullptr);

    // Результат (устанавливается до accept()).
    enum Action {
        NoAction,
        GuardConfirmInDb,           // охранник: да, это <person>
        GuardConfirmNotInDb,        // охранник: реально нет в базе
        GuardItemsRemoved,          // охранник: изъял предмет, повторить проверку
        GuardRejectEntry,           // охранник: отказать
        MedicConfirmBad,            // медик: плохо
        MedicConfirmOk,             // медик: норма
    };

    Action action() const { return action_; }
    Person selectedPerson() const { return selectedPerson_; }

private:
    void buildGuardIdentity(const std::vector<float> &embedding, FaceMatcher *matcher);
    void buildGuardMetal(const QString &message);
    void buildMedic(const QString &message, const QString &badLabel, const QString &okLabel);

    Action action_ = NoAction;
    Person selectedPerson_;
    QListWidget *candidateList_ = nullptr;
    QVector<MatchResult> candidates_;
};

}   // namespace fc
