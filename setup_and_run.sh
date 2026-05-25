#!/bin/bash
# FaceControl — автоматическая установка и запуск (macOS)
# Запуск: ./setup_and_run.sh

set -e

ROOT="$(cd "$(dirname "$0")" && pwd)"
DEPS="$ROOT/deps"
BUILD="$ROOT/build"
MODELS="$ROOT/models"

OPENCV_VERSION="4.13.0"

GREEN='\033[0;32m'; CYAN='\033[0;36m'; YELLOW='\033[1;33m'; RED='\033[0;31m'; NC='\033[0m'
info()  { echo -e "${CYAN}  $*${NC}"; }
ok()    { echo -e "${GREEN}  OK  $*${NC}"; }
warn()  { echo -e "${YELLOW}  !!  $*${NC}"; }
fatal() { echo -e "${RED}\nОШИБКА: $*${NC}"; exit 1; }

echo ""
echo -e "${CYAN}══════════════════════════════════════════${NC}"
echo -e "${CYAN}   FaceControl — Установка и запуск       ${NC}"
echo -e "${CYAN}══════════════════════════════════════════${NC}"
echo ""

# ── 1. Ищем cmake ──────────────────────────────────────────────────────────────
CMAKE=""
for candidate in \
    "$HOME/Qt/Tools/CMake/CMake.app/Contents/bin/cmake" \
    "/Applications/CMake.app/Contents/bin/cmake" \
    "$(which cmake 2>/dev/null)"; do
    [ -x "$candidate" ] && { CMAKE="$candidate"; break; }
done
[ -z "$CMAKE" ] && fatal "cmake не найден. Установите Qt (qt.io) или Homebrew cmake."
ok "cmake: $CMAKE"

# ── 2. Ищем Qt ─────────────────────────────────────────────────────────────────
QT_DIR=""
for candidate in \
    "$HOME/Qt/6.11.0/macos" \
    "$HOME/Qt/6.10.0/macos" \
    "$HOME/Qt/6.9.0/macos"  \
    "$HOME/Qt/6.8.0/macos"  \
    "$HOME/Qt/6.7.0/macos"; do
    [ -f "$candidate/lib/cmake/Qt6/Qt6Config.cmake" ] && { QT_DIR="$candidate"; break; }
done
[ -z "$QT_DIR" ] && fatal "Qt 6 не найден в ~/Qt/. Установите с https://qt.io/download"
ok "Qt: $QT_DIR"

MACDEPLOYQT="$QT_DIR/bin/macdeployqt"

# ── 3. OpenCV ──────────────────────────────────────────────────────────────────
info "Проверяю OpenCV..."

# Ищем в conda/brew/системных путях
OPENCV_ROOT=""
for candidate in \
    "$HOME/miniforge3" "$HOME/miniconda3" "$HOME/opt/miniconda3" \
    "/opt/homebrew" "/usr/local"; do
    [ -f "$candidate/lib/cmake/opencv4/OpenCVConfig.cmake" ] && { OPENCV_ROOT="$candidate"; break; }
    [ -f "$candidate/lib/cmake/OpenCV/OpenCVConfig.cmake"  ] && { OPENCV_ROOT="$candidate"; break; }
done

if [ -z "$OPENCV_ROOT" ]; then
    warn "OpenCV не найден. Устанавливаю через conda (Miniforge3)..."
    # Проверяем наличие conda
    CONDA=""
    for c in "$HOME/miniforge3/bin/conda" "$HOME/miniconda3/bin/conda" \
              "$HOME/opt/miniconda3/bin/conda" "$(which conda 2>/dev/null)"; do
        [ -x "$c" ] && { CONDA="$c"; break; }
    done

    if [ -z "$CONDA" ]; then
        warn "conda не найдена. Скачиваю Miniforge3..."
        ARCH=$(uname -m)
        MF_URL="https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-MacOSX-${ARCH}.sh"
        curl -L "$MF_URL" -o /tmp/miniforge.sh
        bash /tmp/miniforge.sh -b -p "$HOME/miniforge3"
        rm /tmp/miniforge.sh
        CONDA="$HOME/miniforge3/bin/conda"
    fi

    "$CONDA" install -y -c conda-forge "opencv>=$OPENCV_VERSION" cmake || true
    CONDA_BASE=$("$CONDA" info --base)
    OPENCV_ROOT="$CONDA_BASE"
fi
ok "OpenCV root: $OPENCV_ROOT"

# ── 4. Нейросетевые модели ─────────────────────────────────────────────────────
info "Проверяю модели InsightFace..."
mkdir -p "$MODELS"
if [ ! -f "$MODELS/yunet.onnx" ] || [ ! -f "$MODELS/arcface.onnx" ]; then
    warn "Модели не найдены, скачиваю buffalo_l (~170 МБ)..."
    curl -L \
        "https://github.com/deepinsight/insightface/releases/download/v0.7/buffalo_l.zip" \
        -o "$MODELS/buffalo_l.zip"
    unzip -o "$MODELS/buffalo_l.zip" -d "$MODELS/buffalo_l_tmp"
    rm "$MODELS/buffalo_l.zip"
    find "$MODELS/buffalo_l_tmp" -name "det_10g.onnx"   | head -1 | xargs -I{} cp {} "$MODELS/yunet.onnx"
    find "$MODELS/buffalo_l_tmp" -name "w600k_r50.onnx" | head -1 | xargs -I{} cp {} "$MODELS/arcface.onnx"
    rm -rf "$MODELS/buffalo_l_tmp"
fi
[ -f "$MODELS/yunet.onnx"   ] && ok "yunet.onnx"   || warn "yunet.onnx не скачан — распознавание лиц отключено"
[ -f "$MODELS/arcface.onnx" ] && ok "arcface.onnx" || warn "arcface.onnx не скачан — распознавание лиц отключено"

# ── 6. Сборка ──────────────────────────────────────────────────────────────────
APP="$BUILD/FaceControl.app"
BUNDLE_BIN="$APP/Contents/MacOS/FaceControl"
RAW_BIN="$BUILD/facecontrol"

if [ ! -f "$BUNDLE_BIN" ] || [ "${1}" = "--rebuild" ]; then
    info "Конфигурирую проект..."
    mkdir -p "$BUILD"

    OPENCV_CMAKE=""
    for p in \
        "$OPENCV_ROOT/lib/cmake/opencv4" \
        "$OPENCV_ROOT/lib/cmake/OpenCV"; do
        [ -f "$p/OpenCVConfig.cmake" ] && { OPENCV_CMAKE="$p"; break; }
    done
    [ -z "$OPENCV_CMAKE" ] && fatal "OpenCVConfig.cmake не найден в $OPENCV_ROOT"

    "$CMAKE" -S "$ROOT" -B "$BUILD" \
        -DCMAKE_PREFIX_PATH="$QT_DIR" \
        -DOpenCV_DIR="$OPENCV_CMAKE" \
        -DCMAKE_BUILD_TYPE=Release \
        -GNinja

    info "Собираю проект (2-5 минут)..."
    "$CMAKE" --build "$BUILD" -j"$(sysctl -n hw.logicalcpu)"

    # Создаём .app bundle
    info "Создаю .app bundle..."
    if [ -f "$RAW_BIN" ]; then
        mkdir -p "$APP/Contents/MacOS"
        mkdir -p "$APP/Contents/Resources"

        # Info.plist
        cat > "$APP/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
  <key>CFBundleExecutable</key><string>FaceControl</string>
  <key>CFBundleIdentifier</key><string>com.facecontrol.app</string>
  <key>CFBundleName</key><string>FaceControl</string>
  <key>CFBundleVersion</key><string>1.0</string>
  <key>NSCameraUsageDescription</key><string>Камера нужна для распознавания лиц</string>
  <key>NSMicrophoneUsageDescription</key><string>Не используется</string>
  <key>NSHighResolutionCapable</key><true/>
</dict></plist>
PLIST

        cp "$RAW_BIN" "$BUNDLE_BIN"

        # Исправляем RPATH (убираем абсолютные пути conda/Qt, добавляем bundle-relative)
        install_name_tool \
            -delete_rpath "$HOME/miniforge3/lib" 2>/dev/null || true
        install_name_tool \
            -delete_rpath "$QT_DIR/lib" 2>/dev/null || true
        install_name_tool \
            -add_rpath "@loader_path/../Frameworks" "$BUNDLE_BIN" 2>/dev/null || true

        # macdeployqt
        "$MACDEPLOYQT" "$APP" 2>/dev/null || true

        # Убираем дублирующие libQt6*.dylib (конфликт с .framework)
        for f in libQt6Core libQt6Gui libQt6Widgets libQt6DBus libQt6Test; do
            rm -f "$APP/Contents/Frameworks/${f}.6.dylib"
        done

        # Патчим OpenCV модули с Qt-зависимостями
        for lib in "$APP/Contents/Frameworks"/libopencv_highgui*.dylib \
                   "$APP/Contents/Frameworks"/libopencv_cvv*.dylib; do
            [ -f "$lib" ] || continue
            for qt_mod in Core Gui Widgets Test DBus; do
                OLD="@rpath/libQt6${qt_mod}.6.dylib"
                NEW="@rpath/Qt${qt_mod}.framework/Versions/A/Qt${qt_mod}"
                install_name_tool -change "$OLD" "$NEW" "$lib" 2>/dev/null || true
            done
        done

        # Копируем модели в bundle
        mkdir -p "$APP/Contents/MacOS/models"
        [ -f "$MODELS/yunet.onnx"   ] && cp "$MODELS/yunet.onnx"   "$APP/Contents/MacOS/models/"
        [ -f "$MODELS/arcface.onnx" ] && cp "$MODELS/arcface.onnx" "$APP/Contents/MacOS/models/"

    fi
    ok "Сборка завершена!"
else
    ok "Уже собрано (запустите с --rebuild для пересборки)"
fi

# ── 7. Запуск ──────────────────────────────────────────────────────────────────
echo ""
echo -e "${GREEN}══════════════════════════════════════════${NC}"
echo -e "${GREEN}   Запускаю FaceControl...                ${NC}"
echo -e "${GREEN}══════════════════════════════════════════${NC}"
echo ""
echo -e "  Пароль администратора по умолчанию: ${CYAN}admin${NC}"
echo ""

if [ -d "$APP" ]; then
    open "$APP"
elif [ -f "$RAW_BIN" ]; then
    "$RAW_BIN" &
else
    fatal "Исполняемый файл не найден после сборки."
fi
