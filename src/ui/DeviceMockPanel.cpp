#include "DeviceMockPanel.h"

#include "hardware/MockBreathalyzer.h"
#include "hardware/MockMetalDetector.h"
#include "hardware/MockThermalSensor.h"
#include "hardware/MockTurnstile.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace fc {

DeviceMockPanel::DeviceMockPanel(QWidget *parent)
    : QGroupBox(tr("Пульт симулятора"), parent) {
    setMinimumWidth(220);
    setStyleSheet("QGroupBox { font-weight:bold; border:1px solid #555; "
                  "border-radius:6px; margin-top:8px; }"
                  "QGroupBox::title { subcontrol-origin:margin; left:8px; }");

    auto *lay = new QVBoxLayout(this);

    // --- Тепловизор ---
    auto *grpTemp = new QGroupBox(tr("Тепловизор"), this);
    auto *fTemp = new QFormLayout(grpTemp);
    tempSpin_ = new QDoubleSpinBox(grpTemp);
    tempSpin_->setRange(30.0, 45.0);
    tempSpin_->setSingleStep(0.1);
    tempSpin_->setDecimals(1);
    tempSpin_->setValue(36.6);
    tempSpin_->setSuffix(" °C");
    fTemp->addRow(tr("Температура:"), tempSpin_);
    lay->addWidget(grpTemp);

    // --- Металлодетектор ---
    auto *grpMetal = new QGroupBox(tr("Металлодетектор"), this);
    auto *fMetal = new QFormLayout(grpMetal);
    metalCheck_ = new QCheckBox(tr("Срабатывание"), grpMetal);
    metalItems_ = new QLineEdit(grpMetal);
    metalItems_->setPlaceholderText(tr("нож, телефон, ..."));
    fMetal->addRow(metalCheck_);
    fMetal->addRow(tr("Предметы:"), metalItems_);
    lay->addWidget(grpMetal);

    // --- Алкотестер ---
    auto *grpAlc = new QGroupBox(tr("Алкотестер/наркотесты"), this);
    auto *fAlc = new QFormLayout(grpAlc);
    bacSpin_ = new QDoubleSpinBox(grpAlc);
    bacSpin_->setRange(0.0, 3.0);
    bacSpin_->setSingleStep(0.01);
    bacSpin_->setDecimals(2);
    bacSpin_->setSuffix(" ‰");
    narcSpin_ = new QDoubleSpinBox(grpAlc);
    narcSpin_->setRange(0.0, 1.0);
    narcSpin_->setSingleStep(0.01);
    narcSpin_->setDecimals(2);
    narcSpin_->setSuffix(" (0–1)");
    fAlc->addRow(tr("BAC:"), bacSpin_);
    fAlc->addRow(tr("Наркотики:"), narcSpin_);
    lay->addWidget(grpAlc);

    // --- Турникет ---
    auto *grpGate = new QGroupBox(tr("Турникет"), this);
    auto *fGate = new QVBoxLayout(grpGate);
    turnstileStatus_ = new QLabel(tr("Закрыт"), grpGate);
    turnstileStatus_->setAlignment(Qt::AlignCenter);
    turnstileStatus_->setStyleSheet("font-size:14px;font-weight:bold;color:#888");
    passedBtn_ = new QPushButton(tr("👤 Человек прошёл"), grpGate);
    passedBtn_->setStyleSheet("padding:6px;background:#1a4a7a;color:white;border-radius:4px");
    fGate->addWidget(turnstileStatus_);
    fGate->addWidget(passedBtn_);
    lay->addWidget(grpGate);

    lay->addStretch();

    // Подключаем сигналы изменений значений к устройствам при bind.
    connect(tempSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v) {
                if (thermal_) thermal_->setSimulatedTemperature(v);
            });
    connect(metalCheck_, &QCheckBox::toggled, this, [this](bool v) {
        if (metal_) {
            metal_->setSimulatedTriggered(v);
            if (!v) metal_->setSimulatedItems({});
        }
    });
    connect(metalItems_, &QLineEdit::textChanged, this, [this](const QString &t) {
        if (metal_) {
            QStringList items;
            for (const auto &s : t.split(',', Qt::SkipEmptyParts)) {
                const QString trimmed = s.trimmed();
                if (!trimmed.isEmpty()) items << trimmed;
            }
            metal_->setSimulatedItems(items);
            if (!items.isEmpty()) metalCheck_->setChecked(true);
        }
    });
    connect(bacSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v) {
                if (breath_) breath_->setSimulatedBac(v);
            });
    connect(narcSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v) {
                if (breath_) breath_->setSimulatedNarcotic(v);
            });
    connect(passedBtn_, &QPushButton::clicked, this, [this] {
        if (turnstile_) turnstile_->notifyPassed();
    });
}

void DeviceMockPanel::bindDevices(IThermalSensor *thermal,
                                  IMetalDetector *metal,
                                  IBreathalyzer  *breath,
                                  ITurnstile     *turnstile) {
    thermal_  = qobject_cast<MockThermalSensor*>(thermal);
    metal_    = qobject_cast<MockMetalDetector*>(metal);
    breath_   = qobject_cast<MockBreathalyzer*>(breath);
    turnstile_= qobject_cast<MockTurnstile*>(turnstile);

    if (turnstile_) {
        connect(turnstile_, &ITurnstile::stateChanged, this, [this](TurnstileState s) {
            const bool open = (s == TurnstileState::Opened);
            turnstileStatus_->setText(open ? tr("Открыт") : tr("Закрыт"));
            turnstileStatus_->setStyleSheet(
                open ? "font-size:14px;font-weight:bold;color:#4cff4c"
                     : "font-size:14px;font-weight:bold;color:#888");
        });
    }

    // Синхронизируем начальные значения с устройствами.
    if (thermal_) thermal_->setSimulatedTemperature(tempSpin_->value());
    if (breath_)  { breath_->setSimulatedBac(bacSpin_->value()); breath_->setSimulatedNarcotic(narcSpin_->value()); }
}

}   // namespace fc
