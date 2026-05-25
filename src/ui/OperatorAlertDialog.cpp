#include "OperatorAlertDialog.h"
#include "face/FaceMatcher.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace fc {

OperatorAlertDialog::OperatorAlertDialog(CheckpointState waitState,
                                         const QString &message,
                                         FaceMatcher *matcher,
                                         const std::vector<float> &embedding,
                                         QWidget *parent)
    : QDialog(parent) {
    setWindowModality(Qt::WindowModal);
    setMinimumWidth(500);

    auto *lay = new QVBoxLayout(this);

    // Заголовок с иконкой предупреждения.
    auto *hdr = new QHBoxLayout;
    auto *ico = new QLabel("⚠️", this);
    ico->setStyleSheet("font-size:32px");
    auto *msg = new QLabel(message, this);
    msg->setWordWrap(true);
    msg->setStyleSheet("font-size:14px;color:#f0c060");
    hdr->addWidget(ico);
    hdr->addWidget(msg, 1);
    lay->addLayout(hdr);
    lay->addSpacing(8);

    switch (waitState) {
        case CheckpointState::GuardVerifyIdentity:
            setWindowTitle(tr("Вызов охранника — идентификация"));
            buildGuardIdentity(embedding, matcher);
            break;
        case CheckpointState::GuardMetal:
            setWindowTitle(tr("Вызов охранника — металлодетектор"));
            buildGuardMetal(message);
            break;
        case CheckpointState::MedicTemp:
            setWindowTitle(tr("Вызов медика — температура"));
            buildMedic(message, tr("Подтвердить: температура выше нормы"), tr("Температура в норме"));
            break;
        case CheckpointState::MedicAlcohol:
            setWindowTitle(tr("Вызов медика — опьянение"));
            buildMedic(message, tr("Подтвердить: опьянение"), tr("Опьянения нет"));
            break;
        default:
            break;
    }
}

void OperatorAlertDialog::buildGuardIdentity(const std::vector<float> &embedding,
                                              FaceMatcher *matcher) {
    auto *lay = static_cast<QVBoxLayout*>(layout());

    auto *info = new QLabel(tr("Кандидаты из базы (топ-5):"), this);
    lay->addWidget(info);

    candidateList_ = new QListWidget(this);
    lay->addWidget(candidateList_);

    if (matcher && !embedding.empty()) {
        candidates_ = matcher->topK(embedding, 5);
        for (const auto &r : candidates_) {
            const QString label = r.person
                ? QString("%1  [сходство: %2]")
                      .arg(r.person->fullName)
                      .arg(r.similarity, 0, 'f', 3)
                : tr("—");
            candidateList_->addItem(label);
        }
    }

    auto *btnBox = new QHBoxLayout;
    auto *btnFound = new QPushButton(tr("✓ Да, этот человек в базе"), this);
    btnFound->setStyleSheet("background:#1e6b1e;color:white;padding:8px 16px");
    auto *btnNotFound = new QPushButton(tr("✗ Человека нет в базе — отказать"), this);
    btnNotFound->setStyleSheet("background:#7a1a1a;color:white;padding:8px 16px");
    btnBox->addWidget(btnFound);
    btnBox->addWidget(btnNotFound);
    lay->addLayout(btnBox);

    connect(btnFound, &QPushButton::clicked, this, [this] {
        if (!candidateList_) { reject(); return; }
        const int row = candidateList_->currentRow();
        if (row < 0 || row >= candidates_.size()) {
            QMessageBox::warning(this, tr("Выбор"), tr("Выберите человека из списка."));
            return;
        }
        if (!candidates_[row].person) {
            QMessageBox::warning(this, tr("Выбор"), tr("Запись не содержит человека."));
            return;
        }
        selectedPerson_ = *candidates_[row].person;
        action_ = GuardConfirmInDb;
        accept();
    });

    connect(btnNotFound, &QPushButton::clicked, this, [this] {
        action_ = GuardConfirmNotInDb;
        accept();
    });
}

void OperatorAlertDialog::buildGuardMetal(const QString &) {
    auto *lay = static_cast<QVBoxLayout*>(layout());

    auto *btnBox = new QHBoxLayout;
    auto *btnRemoved = new QPushButton(tr("✓ Предмет изъят — повторить проверку"), this);
    btnRemoved->setStyleSheet("background:#1a5a1a;color:white;padding:8px 16px");
    auto *btnReject = new QPushButton(tr("✗ Отказать в допуске"), this);
    btnReject->setStyleSheet("background:#7a1a1a;color:white;padding:8px 16px");
    btnBox->addWidget(btnRemoved);
    btnBox->addWidget(btnReject);
    lay->addLayout(btnBox);

    connect(btnRemoved, &QPushButton::clicked, this, [this] {
        action_ = GuardItemsRemoved;
        accept();
    });
    connect(btnReject, &QPushButton::clicked, this, [this] {
        action_ = GuardRejectEntry;
        accept();
    });
}

void OperatorAlertDialog::buildMedic(const QString &, const QString &badLabel, const QString &okLabel) {
    auto *lay = static_cast<QVBoxLayout*>(layout());

    auto *btnBox = new QHBoxLayout;
    auto *btnBad = new QPushButton(badLabel, this);
    btnBad->setStyleSheet("background:#7a1a1a;color:white;padding:8px 16px");
    auto *btnOk = new QPushButton(okLabel, this);
    btnOk->setStyleSheet("background:#1e6b1e;color:white;padding:8px 16px");
    btnBox->addWidget(btnBad);
    btnBox->addWidget(btnOk);
    lay->addLayout(btnBox);

    connect(btnBad, &QPushButton::clicked, this, [this] {
        action_ = MedicConfirmBad;
        accept();
    });
    connect(btnOk, &QPushButton::clicked, this, [this] {
        action_ = MedicConfirmOk;
        accept();
    });
}

}   // namespace fc
