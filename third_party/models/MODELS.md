# Модели нейросетей

Положите в эту папку следующие файлы:

## `yunet.onnx` — детектор лиц (YuNet)

Скачать из OpenCV Zoo:
```
https://github.com/opencv/opencv_zoo/tree/main/models/face_detection_yunet
```
Нужен файл `face_detection_yunet_2023mar.onnx` — переименуйте в `yunet.onnx`.

## `arcface.onnx` — эмбеддер лиц (ArcFace w600k_r50)

Скачать пакет `buffalo_l` из InsightFace:
```
https://github.com/deepinsight/insightface/tree/master/model_zoo
```
Нужен файл `w600k_r50.onnx` (внутри `buffalo_l.zip`) — переименуйте в `arcface.onnx`.

Ожидаемый формат модели ArcFace: вход `1 x 3 x 112 x 112` (float32), выход `1 x 512`.
