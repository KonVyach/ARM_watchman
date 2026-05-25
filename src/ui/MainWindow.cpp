#include "MainWindow.h"

#include "CheckpointView.h"
#include "admin/AdminLoginDialog.h"
#include "admin/PersonListView.h"
#include "core/CheckpointPipeline.h"
#include "db/PersonRepository.h"
#include "face/FaceEngine.h"
#include "face/FaceMatcher.h"
#include "face/ThresholdPolicy.h"
#include "hardware/DeviceManager.h"

#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>

namespace fc {

MainWindow::MainWindow(CheckpointPipeline *pipeline,
                       FaceEngine         *engine,
                       FaceMatcher        *matcher,
                       ThresholdPolicy    *policy,
                       PersonRepository   *repo,
                       DeviceManager      *devices,
                       QWidget            *parent)
    : QMainWindow(parent),
      pipeline_(pipeline), engine_(engine), matcher_(matcher),
      policy_(policy), repo_(repo), devices_(devices) {

    setWindowTitle(tr("FaceControl — Система контроля доступа"));
    setMinimumSize(1200, 700);

    setStyleSheet(
        "QMainWindow,QWidget{background:#1a1a1a;color:#ddd;}"
        "QTabBar::tab{background:#2a2a2a;color:#aaa;padding:8px 20px;border:1px solid #444;border-bottom:none;border-radius:4px 4px 0 0;}"
        "QTabBar::tab:selected{background:#333;color:#fff;}"
        "QGroupBox{color:#aaa;}"
        "QLabel{color:#ddd;}"
        "QLineEdit,QDoubleSpinBox,QComboBox{background:#2a2a2a;color:#ddd;border:1px solid #555;border-radius:3px;padding:3px;}"
        "QPushButton{background:#333;color:#ddd;border:1px solid #555;border-radius:4px;padding:5px 12px;}"
        "QPushButton:hover{background:#444;}"
        "QTableWidget{background:#222;color:#ddd;gridline-color:#444;}"
        "QHeaderView::section{background:#333;color:#aaa;border:1px solid #444;}"
        "QTextEdit{background:#111;color:#8af;}"
    );

    tabs_ = new QTabWidget(this);
    setCentralWidget(tabs_);

    auto *checkpointView = new CheckpointView(pipeline_, matcher_, devices_, this);
    tabs_->addTab(checkpointView, tr("Контроль доступа"));

    adminTabIndex_ = tabs_->addTab(buildAdminWidget(), tr("🔒 Администрирование"));

    connect(tabs_, &QTabWidget::currentChanged, this, &MainWindow::onTabChanging);

    buildStatusBar();

    // Меню.
    auto *helpMenu = menuBar()->addMenu(tr("Справка"));
    helpMenu->addAction(tr("О программе"), this, [this] {
        QMessageBox::about(this, tr("FaceControl"),
            tr("FaceControl — система автоматизации фейс-контроля.\n\n"
               "Нейросеть: InsightFace ArcFace (buffalo_l)\n"
               "UI: Qt6 / C++17\n"
               "Хранилище: SQLite3"));
    });
}

void MainWindow::onTabChanging(int index) {
    if (index != adminTabIndex_) { adminUnlocked_ = false; return; }
    if (adminUnlocked_) return;

    AdminLoginDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        adminUnlocked_ = true;
    } else {
        // Вернуть на первый таб; блокируем сигнал, чтобы не зациклиться.
        QMetaObject::invokeMethod(tabs_, [this] { tabs_->setCurrentIndex(0); }, Qt::QueuedConnection);
        adminUnlocked_ = false;
    }
}

QWidget *MainWindow::buildAdminWidget() {
    auto *w   = new QWidget(this);
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(12, 12, 12, 4);
    lay->setSpacing(8);

    // --- Группа настроек порога ---
    auto *group = new QGroupBox(tr("Настройки распознавания лица"), w);
    auto *gl    = new QVBoxLayout(group);
    gl->setSpacing(6);

    auto *row = new QHBoxLayout;
    row->addWidget(new QLabel(tr("Порог сходства:"), group));

    auto *slider = new QSlider(Qt::Horizontal, group);
    slider->setRange(0, 100);
    slider->setValue(qRound(policy_->current() * 100));
    slider->setStyleSheet(
        "QSlider::groove:horizontal{height:6px;background:#333;border-radius:3px}"
        "QSlider::sub-page:horizontal{background:#1a6aaa;border-radius:3px}"
        "QSlider::handle:horizontal{width:16px;height:16px;margin:-5px 0;"
        "background:#4aa0e0;border-radius:8px}");
    row->addWidget(slider, 1);

    auto *spin = new QDoubleSpinBox(group);
    spin->setRange(0.01, 0.99);
    spin->setSingleStep(0.01);
    spin->setDecimals(2);
    spin->setValue(policy_->current());
    spin->setFixedWidth(72);
    row->addWidget(spin);

    auto *resetBtn = new QPushButton(tr("По умолчанию"), group);
    resetBtn->setToolTip(tr("Сброс до %1").arg(ThresholdPolicy::kInitial, 0, 'f', 2));
    row->addWidget(resetBtn);
    gl->addLayout(row);

    auto *hint = new QLabel(group);
    hint->setStyleSheet("color:#888;font-size:11px;padding-left:2px");
    gl->addWidget(hint);
    lay->addWidget(group);

    // Описание текущего значения
    auto updateHint = [hint](double v) {
        if (v >= 0.50)
            hint->setText(QObject::tr(
                "Строго (%.2f) — меньше ошибочных допусков, чаще вызов охранника").arg(v));
        else if (v >= 0.38)
            hint->setText(QObject::tr(
                "Стандартно (%.2f) — баланс точности и удобства").arg(v));
        else
            hint->setText(QObject::tr(
                "Мягко (%.2f) — реже вызов охранника, выше риск ложного допуска").arg(v));
    };
    updateHint(policy_->current());

    // Слайдер → спинбокс → политика
    connect(slider, &QSlider::valueChanged, this, [this, spin, updateHint](int v) {
        const double val = v / 100.0;
        spin->blockSignals(true);
        spin->setValue(val);
        spin->blockSignals(false);
        policy_->setValue(val);
        updateHint(val);
    });

    connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, slider, updateHint](double val) {
        slider->blockSignals(true);
        slider->setValue(qRound(val * 100));
        slider->blockSignals(false);
        policy_->setValue(val);
        updateHint(val);
    });

    connect(resetBtn, &QPushButton::clicked, this, [this, slider, spin, updateHint] {
        policy_->reset();
        const double v = policy_->current();
        slider->blockSignals(true); spin->blockSignals(true);
        slider->setValue(qRound(v * 100)); spin->setValue(v);
        slider->blockSignals(false); spin->blockSignals(false);
        updateHint(v);
    });

    // Синхронизация при внешних изменениях (охранник нажал «relax»)
    connect(policy_, &ThresholdPolicy::changed, this, [slider, spin, updateHint](double v) {
        slider->blockSignals(true); spin->blockSignals(true);
        slider->setValue(qRound(v * 100)); spin->setValue(v);
        slider->blockSignals(false); spin->blockSignals(false);
        updateHint(v);
    });

    // --- Список сотрудников ---
    lay->addWidget(new PersonListView(repo_, engine_, w), 1);

    return w;
}

void MainWindow::buildStatusBar() {
    thresholdLabel_ = new QLabel(
        tr("Порог распознавания: %1").arg(policy_->current(), 0, 'f', 2), statusBar());
    thresholdLabel_->setStyleSheet("color:#8af;margin:0 8px");
    statusBar()->addPermanentWidget(thresholdLabel_);

    auto *engineLabel = new QLabel(
        engine_ && engine_->isReady() ? tr("🟢 Нейросеть готова") : tr("🔴 Нейросеть не загружена"),
        statusBar());
    engineLabel->setStyleSheet("margin:0 8px");
    statusBar()->addPermanentWidget(engineLabel);

    connect(policy_, &ThresholdPolicy::changed, this, [this](double v) {
        thresholdLabel_->setText(tr("Порог распознавания: %1").arg(v, 0, 'f', 2));
    });
}

}   // namespace fc
