#pragma once

#include "facecontrol/core/CheckpointState.h"
#include "facecontrol/core/Person.h"
#include "facecontrol/hardware/IBreathalyzer.h"
#include "facecontrol/hardware/IMetalDetector.h"

#include <QWidget>
#include <vector>

class QLabel;
class QPushButton;
class QTextEdit;

namespace fc {

class CameraPreview;
class StepIndicatorWidget;
class CheckpointPipeline;
class FaceMatcher;
class DeviceManager;
class DeviceMockPanel;

class CheckpointView : public QWidget {
    Q_OBJECT
public:
    explicit CheckpointView(CheckpointPipeline *pipeline,
                            FaceMatcher *matcher,
                            DeviceManager *devices,
                            QWidget *parent = nullptr);

private slots:
    void onStateChanged(CheckpointState s);
    void onFaceMatched(const Person &p, double sim);
    void onFaceNotMatched(double sim);
    void onGranted(const Person &p);
    void onDenied(DenyReason reason, const QString &details);
    void onOperatorAction(CheckpointState state, const QString &message);
    void onDiagnostic(const QString &msg);
    void onMetalReading(const MetalDetectorReading &r);
    void onTempReading(double celsius);
    void onIntoxReading(const IntoxicationReading &r);

private:
    void updateStatusBar(const QString &text, const QString &style);
    void appendLog(const QString &text);

    CheckpointPipeline *pipeline_;
    FaceMatcher        *matcher_;
    DeviceManager      *devices_;

    CameraPreview      *facePreview_;
    StepIndicatorWidget*steps_;
    QLabel             *statusLabel_;
    QLabel             *personLabel_;
    QPushButton        *startBtn_;
    QPushButton        *resetBtn_;
    QTextEdit          *logView_;

};

}   // namespace fc
