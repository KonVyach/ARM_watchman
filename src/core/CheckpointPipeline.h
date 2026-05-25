#pragma once

#include "facecontrol/core/CheckpointState.h"
#include "facecontrol/core/Person.h"
#include "facecontrol/hardware/IBreathalyzer.h"
#include "facecontrol/hardware/IMetalDetector.h"
#include "facecontrol/hardware/IThermalSensor.h"
#include "facecontrol/hardware/ITurnstile.h"

#include <QImage>
#include <QObject>
#include <optional>
#include <vector>

namespace fc {

class DeviceManager;
class FaceEngine;
class FaceMatcher;
class ThresholdPolicy;
class PersonRepository;

// Конечный автомат, который проводит человека через все шаги фейс-контроля.
// Все шаги, требующие участия оператора (охранник/медик), переводят автомат
// в «зависшее» состояние ожидания решения — пайплайн ничего не делает, пока
// UI не вызовет соответствующий operator…() метод.
class CheckpointPipeline : public QObject {
    Q_OBJECT
public:
    // Пороговые значения проверок. Меняются из UI / настроек.
    struct Thresholds {
        double maxTemperatureC = 37.2;   // выше — на медика
        double maxBacPromille  = 0.16;   // выше — на медика
        double maxNarcoticScore = 0.30;
    };

    CheckpointPipeline(DeviceManager *devices,
                       FaceEngine *engine,
                       FaceMatcher *matcher,
                       PersonRepository *repo,
                       ThresholdPolicy *policy,
                       QObject *parent = nullptr);

    CheckpointState state() const { return state_; }
    const Thresholds &thresholds() const { return thresholds_; }
    void setThresholds(const Thresholds &t) { thresholds_ = t; }

    // Эмбеддинг лица текущей сессии (используется диалогом охранника).
    const std::vector<float> &currentEmbedding() const { return currentEmbedding_; }

    // --- Внешние команды ---------------------------------------------------
    void start();        // Idle -> FaceCapture
    void reset();        // принудительный возврат в Idle

    // Решения оператора в «модальных» состояниях.
    // GuardVerifyIdentity:
    void guardConfirmInDb(const Person &person);    // охранник опознал, relax порога
    void guardConfirmNotInDb();                     // отказ
    // GuardMetal:
    void guardItemsRemoved();                       // повтор металлодетектора
    void guardRejectMetal();                        // отказ
    // MedicTemp:
    void medicConfirmFever();                       // отказ
    void medicConfirmTempOk();                      // продолжаем
    // MedicAlcohol:
    void medicConfirmIntoxicated();                 // отказ
    void medicConfirmAlcoholOk();                   // допуск

signals:
    void stateChanged(CheckpointState state);
    void facesCaptured(const QImage &face);
    void faceMatched(const Person &person, double similarity);
    void faceNotMatched(double bestSimilarity);
    void metalReadingReceived(const MetalDetectorReading &reading);
    void temperatureReceived(double celsius);
    void intoxicationReceived(const IntoxicationReading &reading);
    void granted(const Person &person);
    void denied(DenyReason reason, const QString &details);
    void operatorActionRequired(CheckpointState state, const QString &message);
    void diagnostic(const QString &message);

private:
    void transition(CheckpointState next);

    // Этапы.
    void doFaceCapture();
    void doFaceMatch();
    void doMetalCheck();
    void doTempCheck();
    void doAlcoholCheck();
    void doGrant();
    void doDeny(DenyReason reason, const QString &details);
    void onTurnstilePassed();

    DeviceManager     *devices_;
    FaceEngine        *engine_;
    FaceMatcher       *matcher_;
    PersonRepository  *repo_;
    ThresholdPolicy   *policy_;

    CheckpointState state_ = CheckpointState::Idle;
    Thresholds      thresholds_;

    // Per-session state.
    QImage              faceFrame_;
    std::vector<float>  currentEmbedding_;
    std::optional<Person> currentPerson_;
    double                lastSimilarity_ = -1.0;
};

}   // namespace fc
