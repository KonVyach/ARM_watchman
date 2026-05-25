#include "CheckpointPipeline.h"

#include "db/EventLog.h"
#include "db/PersonRepository.h"
#include "face/FaceEngine.h"
#include "face/FaceMatcher.h"
#include "face/ThresholdPolicy.h"
#include "hardware/DeviceManager.h"

#include <QDebug>
#include <QTimer>

namespace fc {

CheckpointPipeline::CheckpointPipeline(DeviceManager *devices,
                                       FaceEngine *engine,
                                       FaceMatcher *matcher,
                                       PersonRepository *repo,
                                       ThresholdPolicy *policy,
                                       QObject *parent)
    : QObject(parent),
      devices_(devices), engine_(engine), matcher_(matcher),
      repo_(repo), policy_(policy) {

    // Подписки на устройства.
    connect(devices_->thermal(), &IThermalSensor::temperatureRead,
            this, [this](double c) {
                if (state_ != CheckpointState::TempCheck) return;
                emit temperatureReceived(c);
                if (c > thresholds_.maxTemperatureC) {
                    EventLog::log("temp", "alert", currentPerson_ ? currentPerson_->id : -1,
                                  QString("t=%1").arg(c));
                    transition(CheckpointState::MedicTemp);
                    emit operatorActionRequired(state_,
                        tr("Повышенная температура %1°C — вызовите медика").arg(c, 0, 'f', 1));
                } else {
                    EventLog::log("temp", "ok", currentPerson_ ? currentPerson_->id : -1,
                                  QString("t=%1").arg(c));
                    doAlcoholCheck();
                }
            });

    connect(devices_->metal(), &IMetalDetector::readingReady,
            this, [this](const MetalDetectorReading &r) {
                if (state_ != CheckpointState::MetalCheck) return;
                emit metalReadingReceived(r);
                if (r.triggered) {
                    EventLog::log("metal", "alert",
                                  currentPerson_ ? currentPerson_->id : -1,
                                  r.items.join(", "));
                    transition(CheckpointState::GuardMetal);
                    emit operatorActionRequired(state_,
                        tr("Металлодетектор сработал: %1 — вызовите охранника")
                            .arg(r.items.isEmpty() ? tr("металл/электроника")
                                                   : r.items.join(", ")));
                } else {
                    EventLog::log("metal", "ok", currentPerson_ ? currentPerson_->id : -1, {});
                    doTempCheck();
                }
            });

    connect(devices_->breath(), &IBreathalyzer::readingReady,
            this, [this](const IntoxicationReading &r) {
                if (state_ != CheckpointState::AlcoholCheck) return;
                emit intoxicationReceived(r);
                const bool drunk = (r.bacPromille > thresholds_.maxBacPromille)
                                 || (r.narcoticScore > thresholds_.maxNarcoticScore);
                if (drunk) {
                    EventLog::log("alcohol", "alert",
                                  currentPerson_ ? currentPerson_->id : -1,
                                  QString("bac=%1 narc=%2")
                                      .arg(r.bacPromille).arg(r.narcoticScore));
                    transition(CheckpointState::MedicAlcohol);
                    emit operatorActionRequired(state_,
                        tr("Признаки опьянения (BAC %1‰, наркотики %2) — вызовите медика")
                            .arg(r.bacPromille, 0, 'f', 2)
                            .arg(r.narcoticScore, 0, 'f', 2));
                } else {
                    EventLog::log("alcohol", "ok",
                                  currentPerson_ ? currentPerson_->id : -1,
                                  QString("bac=%1 narc=%2")
                                      .arg(r.bacPromille).arg(r.narcoticScore));
                    doGrant();
                }
            });

    connect(devices_->turnstile(), &ITurnstile::personPassed,
            this, &CheckpointPipeline::onTurnstilePassed);
}

void CheckpointPipeline::transition(CheckpointState next) {
    if (state_ == next) return;
    state_ = next;
    emit stateChanged(state_);
    emit diagnostic(tr("→ %1").arg(state_label(state_)));
}

// ---------------------------------------------------------------------------

void CheckpointPipeline::start() {
    if (state_ != CheckpointState::Idle) return;
    faceFrame_ = QImage();
    currentEmbedding_.clear();
    currentPerson_.reset();
    lastSimilarity_ = -1.0;

    transition(CheckpointState::FaceCapture);
    // Маленькая задержка — даём камерам отдать свежий кадр.
    QTimer::singleShot(150, this, [this] { doFaceCapture(); });
}

void CheckpointPipeline::reset() {
    devices_->turnstile()->close();
    transition(CheckpointState::Idle);
}

// ---------------------------------------------------------------------------

void CheckpointPipeline::doFaceCapture() {
    QImage face = devices_->faceCamera() ? devices_->faceCamera()->lastFrame() : QImage();

    if (face.isNull()) {
        emit diagnostic(tr("Камера лица не отдала кадр — повторяю..."));
        QTimer::singleShot(200, this, [this] { doFaceCapture(); });
        return;
    }
    faceFrame_ = face;
    emit facesCaptured(faceFrame_);

    transition(CheckpointState::FaceMatch);
    doFaceMatch();
}

void CheckpointPipeline::doFaceMatch() {
    if (!engine_ || !engine_->isReady()) {
        emit diagnostic(tr("Нейросеть не загружена — пропускаем шаг сравнения."));
        // Без нейросети сразу зовём охранника.
        transition(CheckpointState::GuardVerifyIdentity);
        emit operatorActionRequired(state_,
            tr("Нейросеть недоступна — опознайте человека вручную."));
        return;
    }

    currentEmbedding_ = engine_->embedLargestFace(faceFrame_);
    if (currentEmbedding_.empty()) {
        emit diagnostic(tr("Лицо не обнаружено на кадре."));
        transition(CheckpointState::GuardVerifyIdentity);
        emit operatorActionRequired(state_,
            tr("Не удалось получить скан лица — вмешайтесь, охранник."));
        return;
    }

    auto result = matcher_->findBest(currentEmbedding_);
    lastSimilarity_ = result.similarity;

    if (result.person) {
        if (result.person->status == PersonStatus::Blacklisted) {
            EventLog::log("face", "blacklist", result.person->id,
                          QString("sim=%1").arg(result.similarity));
            currentPerson_ = result.person;
            doDeny(DenyReason::Blacklisted, tr("Человек в чёрном списке"));
            return;
        }
        currentPerson_ = result.person;
        emit faceMatched(*result.person, result.similarity);
        EventLog::log("face", "match", result.person->id,
                      QString("sim=%1").arg(result.similarity));
        doMetalCheck();
    } else {
        emit faceNotMatched(result.similarity);
        EventLog::log("face", "no_match", -1,
                      QString("best=%1 threshold=%2")
                          .arg(result.similarity)
                          .arg(policy_->current()));
        transition(CheckpointState::GuardVerifyIdentity);
        emit operatorActionRequired(state_,
            tr("Лицо не опознано (макс. сходство %1) — подойдите, охранник.")
                .arg(result.similarity, 0, 'f', 2));
    }
}

// ---------------------------------------------------------------------------

void CheckpointPipeline::guardConfirmInDb(const Person &person) {
    if (state_ != CheckpointState::GuardVerifyIdentity) return;
    currentPerson_ = person;
    // Понижаем строгость, чтобы в следующий раз сеть распознала сама.
    policy_->relax();
    emit diagnostic(tr("Охранник подтвердил: %1 в базе. Порог снижен до %2.")
                       .arg(person.fullName)
                       .arg(policy_->current(), 0, 'f', 2));
    EventLog::log("face", "operator_match", person.id,
                  QString("relaxed_to=%1").arg(policy_->current()));
    if (person.status == PersonStatus::Blacklisted) {
        doDeny(DenyReason::Blacklisted, tr("Человек в чёрном списке"));
        return;
    }
    doMetalCheck();
}

void CheckpointPipeline::guardConfirmNotInDb() {
    if (state_ != CheckpointState::GuardVerifyIdentity) return;
    EventLog::log("face", "operator_not_in_db", -1, {});
    doDeny(DenyReason::NotInDatabase, tr("Охранник подтвердил: человека нет в базе"));
}

// ---------------------------------------------------------------------------

void CheckpointPipeline::doMetalCheck() {
    transition(CheckpointState::MetalCheck);
    devices_->metal()->requestReading();
}

void CheckpointPipeline::guardItemsRemoved() {
    if (state_ != CheckpointState::GuardMetal) return;
    EventLog::log("metal", "items_removed",
                  currentPerson_ ? currentPerson_->id : -1, {});
    doMetalCheck();
}

void CheckpointPipeline::guardRejectMetal() {
    if (state_ != CheckpointState::GuardMetal) return;
    EventLog::log("metal", "operator_reject",
                  currentPerson_ ? currentPerson_->id : -1, {});
    doDeny(DenyReason::ProhibitedItems, tr("Охранник: запрещённые предметы"));
}

// ---------------------------------------------------------------------------

void CheckpointPipeline::doTempCheck() {
    transition(CheckpointState::TempCheck);
    devices_->thermal()->requestReading();
}

void CheckpointPipeline::medicConfirmFever() {
    if (state_ != CheckpointState::MedicTemp) return;
    EventLog::log("temp", "medic_fever",
                  currentPerson_ ? currentPerson_->id : -1, {});
    doDeny(DenyReason::HighTemperature, tr("Медик подтвердил повышенную температуру"));
}

void CheckpointPipeline::medicConfirmTempOk() {
    if (state_ != CheckpointState::MedicTemp) return;
    EventLog::log("temp", "medic_ok",
                  currentPerson_ ? currentPerson_->id : -1, {});
    doAlcoholCheck();
}

// ---------------------------------------------------------------------------

void CheckpointPipeline::doAlcoholCheck() {
    transition(CheckpointState::AlcoholCheck);
    devices_->breath()->requestReading();
}

void CheckpointPipeline::medicConfirmIntoxicated() {
    if (state_ != CheckpointState::MedicAlcohol) return;
    EventLog::log("alcohol", "medic_drunk",
                  currentPerson_ ? currentPerson_->id : -1, {});
    doDeny(DenyReason::Intoxicated, tr("Медик подтвердил опьянение"));
}

void CheckpointPipeline::medicConfirmAlcoholOk() {
    if (state_ != CheckpointState::MedicAlcohol) return;
    EventLog::log("alcohol", "medic_ok",
                  currentPerson_ ? currentPerson_->id : -1, {});
    doGrant();
}

// ---------------------------------------------------------------------------

void CheckpointPipeline::doGrant() {
    transition(CheckpointState::Granted);
    devices_->turnstile()->open();
    Person p = currentPerson_.value_or(Person{});
    EventLog::log("gate", "granted", p.id, p.fullName);
    emit granted(p);
}

void CheckpointPipeline::doDeny(DenyReason reason, const QString &details) {
    transition(CheckpointState::Denied);
    devices_->turnstile()->close();
    EventLog::log("gate", "denied",
                  currentPerson_ ? currentPerson_->id : -1,
                  QString("%1: %2").arg(deny_reason_label(reason), details));
    emit denied(reason, details);
    // Возврат в Idle произойдёт после нажатия «Сброс» в UI или таймера.
    QTimer::singleShot(5000, this, [this] {
        if (state_ == CheckpointState::Denied) reset();
    });
}

void CheckpointPipeline::onTurnstilePassed() {
    if (state_ != CheckpointState::Granted) return;
    EventLog::log("gate", "passed", currentPerson_ ? currentPerson_->id : -1, {});
    devices_->turnstile()->close();
    transition(CheckpointState::Idle);
}

}   // namespace fc
