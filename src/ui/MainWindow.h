#pragma once

#include <QMainWindow>

class QTabWidget;
class QLabel;

namespace fc {

class CheckpointPipeline;
class FaceEngine;
class FaceMatcher;
class ThresholdPolicy;
class PersonRepository;
class DeviceManager;
class CheckpointView;
class PersonListView;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(CheckpointPipeline *pipeline,
               FaceEngine         *engine,
               FaceMatcher        *matcher,
               ThresholdPolicy    *policy,
               PersonRepository   *repo,
               DeviceManager      *devices,
               QWidget *parent = nullptr);

private slots:
    void onTabChanging(int index);

private:
    void buildStatusBar();
    QWidget *buildAdminWidget();

    CheckpointPipeline *pipeline_;
    FaceEngine         *engine_;
    FaceMatcher        *matcher_;
    ThresholdPolicy    *policy_;
    PersonRepository   *repo_;
    DeviceManager      *devices_;

    QTabWidget         *tabs_           = nullptr;
    int                 adminTabIndex_  = 1;
    bool                adminUnlocked_  = false;

    QLabel             *thresholdLabel_ = nullptr;
};

}   // namespace fc
