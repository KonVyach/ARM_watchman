#pragma once

#include <QImage>
#include <QObject>
#include <QString>
#include <memory>
#include <vector>

namespace fc {

struct DetectedFace {
    int x = 0, y = 0, w = 0, h = 0;
    float score = 0.f;
};

// Объединяет детектор лиц (OpenCV FaceDetectorYN) и нейросеть-эмбеддер ArcFace
// (через ONNX Runtime). Скрывает всю работу с моделями за двумя функциями:
// detect() и embed(). Используется и при enrollment-е, и при контроле.
class FaceEngine : public QObject {
    Q_OBJECT
public:
    explicit FaceEngine(QObject *parent = nullptr);
    ~FaceEngine() override;

    // Инициализация: пути к .onnx моделям. yunetModelPath — детектор лиц,
    // arcfaceModelPath — модель эмбеддера (вход 1x3x112x112, выход 1x512).
    bool initialize(const QString &yunetModelPath,
                    const QString &arcfaceModelPath,
                    QString *errorOut = nullptr);
    bool isReady() const;

    // Детекция: ищет лица на кадре. Возвращает список боксов и (внутренне)
    // запоминает 5 ключевых точек самого крупного лица — они нужны для embed().
    std::vector<DetectedFace> detect(const QImage &frame);

    // Эмбеддинг для самого крупного обнаруженного лица. Кадр пересылается
    // повторно для удобства. Возвращает 512-D L2-normalised вектор. Пустой
    // вектор означает «не удалось» (нет лица или ошибка).
    std::vector<float> embedLargestFace(const QImage &frame);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}   // namespace fc
