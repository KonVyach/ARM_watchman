#include "core/CheckpointPipeline.h"
#include "db/Database.h"
#include "db/PersonRepository.h"
#include "face/FaceEngine.h"
#include "face/FaceMatcher.h"
#include "face/ThresholdPolicy.h"
#include "hardware/DeviceManager.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("FaceControl");
    app.setOrganizationName("FaceControl");

    // --- База данных ---
    const QString dataDir = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    const QString dbPath = dataDir + "/facecontrol.db";
    if (!fc::Database::open(dbPath)) {
        QMessageBox::critical(nullptr, "FaceControl",
            QStringLiteral("Не удалось открыть базу данных:\n%1").arg(dbPath));
        return 1;
    }

    // --- Компоненты ---
    auto *repo    = new fc::PersonRepository;
    auto *policy  = new fc::ThresholdPolicy;
    policy->load();

    auto *engine  = new fc::FaceEngine;
    auto *matcher = new fc::FaceMatcher(repo, policy);
    auto *devices = fc::DeviceManager::createDefault();

    // --- Загрузка нейросетевых моделей ---
    // Ищем модели рядом с исполняемым файлом, затем в third_party/models.
    const QString exeDir = QCoreApplication::applicationDirPath();
    const QStringList modelDirs = {
        exeDir + "/models",
        exeDir + "/../third_party/models",
        exeDir + "/third_party/models",
    };
    QString yunetPath, arcfacePath;
    for (const auto &d : modelDirs) {
        const QString yp = d + "/yunet.onnx";
        const QString ap = d + "/arcface.onnx";
        if (QDir().exists(yp) && QDir().exists(ap)) {
            yunetPath   = yp;
            arcfacePath = ap;
            break;
        }
    }

    if (!yunetPath.isEmpty()) {
        QString err;
        if (!engine->initialize(yunetPath, arcfacePath, &err)) {
            qWarning() << "FaceEngine init failed:" << err;
            QMessageBox::warning(nullptr, "FaceControl",
                QStringLiteral("Нейросеть не загружена:\n%1\n\n"
                               "Работа без автоматического распознавания лиц. "
                               "Охраннику придётся идентифицировать всех вручную.").arg(err));
        }
    } else {
        qWarning() << "ONNX models not found — face engine disabled";
    }

    // --- Пайплайн ---
    fprintf(stderr, "[main] creating pipeline\n"); fflush(stderr);
    auto *pipeline = new fc::CheckpointPipeline(devices, engine, matcher, repo, policy);
    fprintf(stderr, "[main] pipeline ok\n"); fflush(stderr);

    // --- Камеры: стартуем захват ---
    if (devices->faceCamera()) devices->faceCamera()->start();
    fprintf(stderr, "[main] cameras started\n"); fflush(stderr);

    // --- Главное окно ---
    fprintf(stderr, "[main] creating MainWindow\n"); fflush(stderr);
    fc::MainWindow window(pipeline, engine, matcher, policy, repo, devices);
    fprintf(stderr, "[main] MainWindow constructed\n"); fflush(stderr);
    window.move(100, 100);
    window.resize(1200, 750);
    window.show();
    window.raise();
    window.activateWindow();
    fprintf(stderr, "[main] shown geo=%dx%d+%d+%d\n",
        window.width(), window.height(), window.x(), window.y()); fflush(stderr);

    const int ret = app.exec();

    if (devices->faceCamera()) devices->faceCamera()->stop();

    return ret;
}
