# FaceControl

Desktop application for automating face-control / access-control at the entrance of an
institution. Walks a person through a configurable chain of checks: face recognition →
metal detector → temperature → alcohol/intoxication → turnstile.

Hardware (thermal imager, metal detector, breathalyzer, turnstile) is abstracted behind
small C++ interfaces and ships with software mocks driven from a control panel in the
UI. Web-cameras are real (OpenCV `VideoCapture`).

## Stack

- C++17 + Qt6 (Widgets, Sql, Multimedia)
- OpenCV 4 — camera capture + face detection (`cv::FaceDetectorYN`)
- ONNX Runtime (C++) — ArcFace face embeddings
- SQLite3 — local people DB and audit log

## Build

```bash
# 1. Get ONNX Runtime (prebuilt, any 1.16+ release):
#    https://github.com/microsoft/onnxruntime/releases
#    Extract somewhere, e.g. ~/onnxruntime-osx-arm64-1.18.0/

# 2. Get OpenCV (with the face_detection_yunet model):
#    macOS:  brew install opencv
#    Ubuntu: apt install libopencv-dev

# 3. Get the InsightFace ArcFace model (one-time):
#    Download `buffalo_l` from https://github.com/deepinsight/insightface
#    Place `w600k_r50.onnx` into third_party/models/arcface.onnx
#    Place a YuNet detector model into third_party/models/yunet.onnx
#    (e.g. https://github.com/opencv/opencv_zoo/tree/main/models/face_detection_yunet)

# 4. Configure & build
cmake -S . -B build \
    -DONNXRUNTIME_DIR=$HOME/onnxruntime-osx-arm64-1.18.0
cmake --build build -j

# 5. Run
./build/facecontrol
```

## First run

- An empty SQLite database is created at `~/.facecontrol/facecontrol.db`.
- Default admin password is `admin` — change it on first login.
- Open the **Администрирование** tab → log in → **Добавить человека** to enroll people.
- Switch to the **Контроль** tab to run the checkpoint.

## Hardware mocks

The right-hand **Пульт симулятора** lets you drive every non-camera device by hand:
- temperature slider (°C),
- metal-detector trigger checkbox,
- BAC and narcotic-score sliders,
- "human passed through turnstile" button.

To plug in real hardware, implement the corresponding interface in
`include/facecontrol/hardware/` (`IThermalSensor`, `IMetalDetector`, `IBreathalyzer`,
`ITurnstile`) and swap the registration in `DeviceManager::createDefault`.

## Notes on the adaptive threshold

The face matcher starts with cosine-similarity threshold `0.45`. Every time a guard
confirms that a face the model failed to recognise was *actually* in the database, the
threshold is relaxed by `0.02`, down to a floor of `0.30`. The current value is stored
in the `settings` table.
