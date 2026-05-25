cmake_minimum_required(VERSION 3.20)
project(facecontrol LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# --- Dependencies --------------------------------------------------------
find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Widgets Sql Multimedia MultimediaWidgets)
find_package(OpenCV 4 REQUIRED)
# SQLite используется через Qt SQL (QSQLITE driver встроен в Qt) — отдельная
# системная библиотека не нужна.

# ArcFace inference is done via cv::dnn::readNetFromONNX (OpenCV DNN module).
# No separate ONNX Runtime library needed — works on Windows (MinGW & MSVC),
# macOS, and Linux without any extra downloads.

# --- Sources -------------------------------------------------------------
set(FC_SOURCES
    src/main.cpp

    src/core/CheckpointPipeline.cpp
    src/core/Person.cpp

    src/hardware/ICamera.cpp
    src/hardware/IThermalSensor.cpp
    src/hardware/IMetalDetector.cpp
    src/hardware/IBreathalyzer.cpp
    src/hardware/ITurnstile.cpp
    src/hardware/WebcamOpenCv.cpp
    src/hardware/MockThermalSensor.cpp
    src/hardware/MockMetalDetector.cpp
    src/hardware/MockBreathalyzer.cpp
    src/hardware/MockTurnstile.cpp
    src/hardware/DeviceManager.cpp

    src/db/Database.cpp
    src/db/PersonRepository.cpp
    src/db/EventLog.cpp

    src/face/FaceEngine.cpp
    src/face/FaceMatcher.cpp
    src/face/ThresholdPolicy.cpp

    src/ui/MainWindow.cpp
    src/ui/CheckpointView.cpp
    src/ui/StepIndicatorWidget.cpp
    src/ui/OperatorAlertDialog.cpp
    src/ui/DeviceMockPanel.cpp
    src/ui/CameraPreview.cpp
    src/ui/admin/AdminLoginDialog.cpp
    src/ui/admin/PersonListView.cpp
    src/ui/admin/PersonEditDialog.cpp
)

# Явная генерация MOC для интерфейсных заголовков из include/ — AUTOMOC не
# обрабатывает их автоматически, т.к. они находятся вне src/.
qt6_wrap_cpp(FC_INTERFACE_MOCS
    include/facecontrol/hardware/ICamera.h
    include/facecontrol/hardware/IThermalSensor.h
    include/facecontrol/hardware/IMetalDetector.h
    include/facecontrol/hardware/IBreathalyzer.h
    include/facecontrol/hardware/ITurnstile.h
    TARGET facecontrol
)

add_executable(facecontrol ${FC_SOURCES} ${FC_INTERFACE_MOCS})

target_include_directories(facecontrol PRIVATE
    include
    src
    src/ui
    src/ui/admin
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(facecontrol PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Multimedia Qt6::MultimediaWidgets
    ${OpenCV_LIBS}
)

# --- Tests ---------------------------------------------------------------
option(FC_BUILD_TESTS "Build unit tests" OFF)
if(FC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
