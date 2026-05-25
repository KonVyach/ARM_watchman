#include "CheckpointView.h"

#include "CameraPreview.h"
#include "DeviceMockPanel.h"
#include "OperatorAlertDialog.h"
#include "StepIndicatorWidget.h"
#include "core/CheckpointPipeline.h"
#include "face/FaceMatcher.h"
#include "hardware/DeviceManager.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

namespace fc {

CheckpointView::CheckpointView(CheckpointPipeline *pipeline,
                               FaceMatcher *matcher,
                               DeviceManager *devices,
                               QWidget *parent)
    : QWidget(parent), pipeline_(pipeline), matcher_(matcher), devices_(devices) {

    auto *mainLayout = new QHBoxLayout(this);

    // --- Левая рабочая часть ---
    auto *leftWidget = new QWidget(this);
    auto *leftLay = new QVBoxLayout(leftWidget);

    // Индикатор шагов.
    steps_ = new StepIndicatorWidget(leftWidget);
    leftLay->addWidget(steps_);

    // Камера.
    facePreview_ = new CameraPreview(tr("Скан лица"), leftWidget);
    leftLay->addWidget(facePreview_);

    if (devices_->faceCamera()) facePreview_->bind(devices_->faceCamera());

    // Статус.
    statusLabel_ = new QLabel(tr("Ожидание"), leftWidget);
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setMinimumHeight(44);
    statusLabel_->setStyleSheet("font-size:18px;background:#222;border-radius:6px;color:#aaa;padding:4px 16px");
    leftLay->addWidget(statusLabel_);

    // Информация о человеке.
    personLabel_ = new QLabel(leftWidget);
    personLabel_->setAlignment(Qt::AlignCenter);
    personLabel_->setStyleSheet("font-size:13px;color:#80c0f0");
    personLabel_->hide();
    leftLay->addWidget(personLabel_);

    // Кнопки.
    auto *btnRow = new QHBoxLayout;
    startBtn_ = new QPushButton(tr("▶ Начать проверку"), leftWidget);
    startBtn_->setStyleSheet("font-size:14px;padding:10px 24px;background:#1a4a7a;color:white;border-radius:6px");
    resetBtn_ = new QPushButton(tr("↺ Сброс"), leftWidget);
    resetBtn_->setStyleSheet("font-size:14px;padding:10px 24px;background:#444;color:white;border-radius:6px");
    btnRow->addStretch();
    btnRow->addWidget(startBtn_);
    btnRow->addWidget(resetBtn_);
    btnRow->addStretch();
    leftLay->addLayout(btnRow);

    // Лог событий.
    logView_ = new QTextEdit(leftWidget);
    logView_->setReadOnly(true);
    logView_->setMaximumHeight(160);
    logView_->setStyleSheet("background:#111;color:#8af;font-family:monospace;font-size:11px");
    leftLay->addWidget(logView_);

    mainLayout->addWidget(leftWidget, 1);

    // --- Правая панель симулятора ---
    auto *mockPanel = new DeviceMockPanel(this);
    mockPanel->bindDevices(devices_->thermal(), devices_->metal(),
                           devices_->breath(), devices_->turnstile());
    mainLayout->addWidget(mockPanel, 0);

    // --- Подключение сигналов пайплайна ---
    connect(pipeline_, &CheckpointPipeline::stateChanged,       this, &CheckpointView::onStateChanged);
    connect(pipeline_, &CheckpointPipeline::faceMatched,        this, &CheckpointView::onFaceMatched);
    connect(pipeline_, &CheckpointPipeline::faceNotMatched,     this, &CheckpointView::onFaceNotMatched);
    connect(pipeline_, &CheckpointPipeline::granted,            this, &CheckpointView::onGranted);
    connect(pipeline_, &CheckpointPipeline::denied,             this, &CheckpointView::onDenied);
    connect(pipeline_, &CheckpointPipeline::operatorActionRequired, this, &CheckpointView::onOperatorAction);
    connect(pipeline_, &CheckpointPipeline::diagnostic,         this, &CheckpointView::onDiagnostic);
    connect(pipeline_, &CheckpointPipeline::metalReadingReceived, this, &CheckpointView::onMetalReading);
    connect(pipeline_, &CheckpointPipeline::temperatureReceived,  this, &CheckpointView::onTempReading);
    connect(pipeline_, &CheckpointPipeline::intoxicationReceived, this, &CheckpointView::onIntoxReading);

    connect(startBtn_, &QPushButton::clicked, pipeline_, &CheckpointPipeline::start);
    connect(resetBtn_, &QPushButton::clicked, pipeline_, &CheckpointPipeline::reset);
}

// ---------------------------------------------------------------------------

void CheckpointView::onStateChanged(CheckpointState s) {
    steps_->setState(s);
    updateStatusBar(state_label(s), "");
    startBtn_->setEnabled(s == CheckpointState::Idle);
    if (s == CheckpointState::Idle) {
        personLabel_->hide();
        updateStatusBar(tr("Ожидание"),
                        "font-size:18px;background:#222;border-radius:6px;color:#aaa;padding:4px 16px");
    }
}

void CheckpointView::onFaceMatched(const Person &p, double sim) {
    personLabel_->setText(tr("✓ %1  (сходство: %2)")
                              .arg(p.fullName).arg(sim, 0, 'f', 3));
    personLabel_->setStyleSheet("font-size:13px;color:#80f080");
    personLabel_->show();
    // Сохраняем эмбеддинг для диалога.
    appendLog(tr("Распознан: %1 (cos=%2)").arg(p.fullName).arg(sim, 0, 'f', 3));
}

void CheckpointView::onFaceNotMatched(double sim) {
    personLabel_->setText(tr("? Не опознан  (макс. сходство: %1)").arg(sim, 0, 'f', 3));
    personLabel_->setStyleSheet("font-size:13px;color:#f09050");
    personLabel_->show();
    appendLog(tr("Лицо не найдено в базе, макс. сходство=%1").arg(sim, 0, 'f', 3));
}

void CheckpointView::onGranted(const Person &p) {
    updateStatusBar(tr("✓ ДОПУСК РАЗРЕШЁН — %1").arg(p.fullName),
                    "font-size:18px;background:#1e6b1e;border-radius:6px;color:#9ef09e;font-weight:bold;padding:4px 16px");
    steps_->setState(CheckpointState::Granted);
    appendLog(tr("✓ ДОПУСК РАЗРЕШЁН: %1").arg(p.fullName));
}

void CheckpointView::onDenied(DenyReason reason, const QString &details) {
    updateStatusBar(tr("✗ ДОПУСК ЗАПРЕЩЁН — %1").arg(deny_reason_label(reason)),
                    "font-size:18px;background:#7a1a1a;border-radius:6px;color:#f08080;font-weight:bold;padding:4px 16px");
    steps_->setState(CheckpointState::Denied, reason);
    appendLog(tr("✗ ОТКАЗ: %1 — %2").arg(deny_reason_label(reason), details));
}

void CheckpointView::onOperatorAction(CheckpointState state, const QString &message) {
    appendLog(tr("⚠ ВЫЗОВ ОПЕРАТОРА [%1]: %2").arg(state_label(state), message));

    OperatorAlertDialog dlg(state, message, matcher_,
                            pipeline_->currentEmbedding(), this);

    if (dlg.exec() == QDialog::Accepted) {
        switch (dlg.action()) {
            case OperatorAlertDialog::GuardConfirmInDb:
                appendLog(tr("Охранник: %1 опознан вручную").arg(dlg.selectedPerson().fullName));
                pipeline_->guardConfirmInDb(dlg.selectedPerson());
                break;
            case OperatorAlertDialog::GuardConfirmNotInDb:
                appendLog(tr("Охранник: подтверждено — не в базе"));
                pipeline_->guardConfirmNotInDb();
                break;
            case OperatorAlertDialog::GuardItemsRemoved:
                appendLog(tr("Охранник: предмет изъят, повтор проверки"));
                pipeline_->guardItemsRemoved();
                break;
            case OperatorAlertDialog::GuardRejectEntry:
                appendLog(tr("Охранник: отказ в допуске"));
                pipeline_->guardRejectMetal();
                break;
            case OperatorAlertDialog::MedicConfirmBad:
                if (state == CheckpointState::MedicTemp) {
                    appendLog(tr("Медик: температура выше нормы"));
                    pipeline_->medicConfirmFever();
                } else {
                    appendLog(tr("Медик: опьянение подтверждено"));
                    pipeline_->medicConfirmIntoxicated();
                }
                break;
            case OperatorAlertDialog::MedicConfirmOk:
                if (state == CheckpointState::MedicTemp) {
                    appendLog(tr("Медик: температура в норме"));
                    pipeline_->medicConfirmTempOk();
                } else {
                    appendLog(tr("Медик: опьянения нет"));
                    pipeline_->medicConfirmAlcoholOk();
                }
                break;
            default: break;
        }
    }
}

void CheckpointView::onDiagnostic(const QString &msg) {
    appendLog(msg);
}

void CheckpointView::onMetalReading(const MetalDetectorReading &r) {
    appendLog(r.triggered
        ? tr("Металл: СРАБАТЫВАНИЕ — %1").arg(r.items.join(", "))
        : tr("Металл: чисто"));
}

void CheckpointView::onTempReading(double celsius) {
    appendLog(tr("Температура: %1°C").arg(celsius, 0, 'f', 1));
}

void CheckpointView::onIntoxReading(const IntoxicationReading &r) {
    appendLog(tr("BAC: %1‰, наркотики: %2")
                  .arg(r.bacPromille, 0, 'f', 2)
                  .arg(r.narcoticScore, 0, 'f', 2));
}

void CheckpointView::updateStatusBar(const QString &text, const QString &style) {
    statusLabel_->setText(text);
    if (!style.isEmpty()) statusLabel_->setStyleSheet(style);
}

void CheckpointView::appendLog(const QString &text) {
    const QString ts = QDateTime::currentDateTime().toString("HH:mm:ss");
    logView_->append(QString("[%1] %2").arg(ts, text));
}

}   // namespace fc
