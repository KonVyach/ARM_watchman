# FaceControl - Auto setup and run (Windows, MSVC)
# Requirements:
#   1. Visual Studio 2022 Build Tools (https://visualstudio.microsoft.com/visual-cpp-build-tools/)
#      - Select: "Desktop development with C++"
#   2. Qt 6.x with MSVC 2022 64-bit component (via Qt Maintenance Tool)
# Then just run this script.

param(
    [string]$QtHint = "",
    [switch]$Rebuild
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

try { Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned -Force } catch { }

$ROOT = Split-Path -Parent $MyInvocation.MyCommand.Path
$DEPS = "$ROOT\deps"
$BUILD = "$ROOT\build_win"
$OPENCV_VERSION = "4.10.0"

function Info  { param($m) Write-Host "  $m" -ForegroundColor Cyan }
function OK    { param($m) Write-Host "  OK  $m" -ForegroundColor Green }
function Warn  { param($m) Write-Host "  !!  $m" -ForegroundColor Yellow }
function Fatal {
    param($m)
    Write-Host "`nERROR: $m" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host ""
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "   FaceControl - Setup and Run            " -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

# --- Check Visual Studio / MSVC ---
Info "Looking for Visual Studio..."
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    Fatal "Visual Studio Build Tools not found.`n`n  Install from: https://visualstudio.microsoft.com/visual-cpp-build-tools/`n  Select workload: Desktop development with C++"
}
$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
if (-not $vsPath) {
    Fatal "C++ tools not found in Visual Studio.`n`n  Open Visual Studio Installer -> Modify -> add 'Desktop development with C++'"
}
$VCVARS = "$vsPath\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $VCVARS)) { Fatal "vcvars64.bat not found at: $VCVARS" }
OK "Visual Studio: $vsPath"

# Load MSVC environment
Info "Loading MSVC environment..."
$vcEnv = cmd /c "`"$VCVARS`" > nul 2>&1 && set" 2>$null
foreach ($line in $vcEnv) {
    if ($line -match "^([^=]+)=(.*)$") {
        [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], "Process")
    }
}
OK "MSVC environment loaded"

# --- Find Qt MSVC ---
Info "Looking for Qt 6 MSVC..."
function Find-Qt-MSVC {
    $bases = @($QtHint, "C:\Qt", "D:\Qt", "$env:USERPROFILE\Qt") | Where-Object { $_ }
    foreach ($base in $bases) {
        if (-not (Test-Path $base)) { continue }
        $hits = Get-ChildItem "$base\*\msvc*\bin\qmake.exe" -ErrorAction SilentlyContinue |
                Sort-Object FullName | Select-Object -Last 1
        if ($hits) { return Split-Path (Split-Path $hits.FullName -Parent) -Parent }
    }
    return $null
}

$QT_DIR = Find-Qt-MSVC
if (-not $QT_DIR) {
    Fatal "Qt 6 MSVC variant not found.`n`n  Open Qt Maintenance Tool -> Add components -> Qt 6.x -> MSVC 2022 64-bit`n  (Qt Maintenance Tool is in your Qt installation folder)"
}
OK "Qt MSVC: $QT_DIR"

$QT_ROOT   = Split-Path (Split-Path $QT_DIR -Parent) -Parent
$CMAKE_EXE = Get-ChildItem "$QT_ROOT\Tools\CMake_64\bin\cmake.exe" -ErrorAction SilentlyContinue |
             Select-Object -First 1 -ExpandProperty FullName
if (-not $CMAKE_EXE) { $CMAKE_EXE = "cmake" }
$WINDEPLOY = "$QT_DIR\bin\windeployqt.exe"

# --- OpenCV ---
$OPENCV_BUILD = "$DEPS\opencv\build"

function Find-OpenCV-CMake {
    foreach ($vc in @("vc17", "vc16", "vc15")) {
        $p = "$OPENCV_BUILD\x64\$vc\lib"
        if (Test-Path "$p\OpenCVConfig.cmake") { return $p }
    }
    return $null
}

Info "Checking OpenCV $OPENCV_VERSION..."
$OPENCV_CMAKE = Find-OpenCV-CMake

if (-not $OPENCV_CMAKE) {
    Warn "OpenCV not found, downloading (~230 MB)..."
    New-Item -ItemType Directory -Force -Path $DEPS | Out-Null
    $installer = "$DEPS\opencv-installer.exe"
    Invoke-WebRequest `
        -Uri "https://github.com/opencv/opencv/releases/download/$OPENCV_VERSION/opencv-$OPENCV_VERSION-windows.exe" `
        -OutFile $installer
    Info "Extracting OpenCV..."
    Start-Process -FilePath $installer -ArgumentList "-o`"$DEPS`" -y" -Wait -NoNewWindow
    Remove-Item $installer -Force -ErrorAction SilentlyContinue
    $OPENCV_CMAKE = Find-OpenCV-CMake
}

if (-not $OPENCV_CMAKE) {
    Fatal "OpenCV cmake config not found after extraction. Expected: $OPENCV_BUILD\x64\vc17\lib"
}
OK "OpenCV: $OPENCV_CMAKE"

# --- Models ---
$MODELS_DIR = "$ROOT\models"
$YUNET      = "$MODELS_DIR\yunet.onnx"
$ARCFACE    = "$MODELS_DIR\arcface.onnx"
$CASCADE    = "$MODELS_DIR\haarcascade_frontalface_default.xml"

New-Item -ItemType Directory -Force -Path $MODELS_DIR | Out-Null

# YuNet face detector (small ~380 KB, from OpenCV Zoo via media.githubusercontent.com)
Info "Checking YuNet face detector..."
if (-not (Test-Path $YUNET)) {
    try {
        Invoke-WebRequest `
            -Uri "https://media.githubusercontent.com/media/opencv/opencv_zoo/main/models/face_detection_yunet/face_detection_yunet_2023mar.onnx" `
            -OutFile $YUNET -TimeoutSec 30
        # Verify it's a real ONNX file (not a Git LFS pointer)
        $bytes = [System.IO.File]::ReadAllBytes($YUNET)
        if ($bytes.Length -lt 10000) { Remove-Item $YUNET -Force; Warn "YuNet download failed (got LFS pointer), using Haar fallback" }
        else { OK "yunet.onnx ($([math]::Round($bytes.Length/1024)) KB)" }
    } catch { Warn "YuNet download failed, using Haar cascade fallback" }
}
if (Test-Path $YUNET) { OK "yunet.onnx" } else { Warn "yunet.onnx missing - will use Haar cascade (slightly less accurate)" }

# ArcFace embedding model (~170 MB from InsightFace buffalo_l)
Info "Checking ArcFace model..."
if (-not (Test-Path $ARCFACE)) {
    Warn "arcface.onnx not found, downloading buffalo_l (~170 MB)..."
    $zipM = "$MODELS_DIR\buffalo_l.zip"
    Invoke-WebRequest `
        -Uri "https://github.com/deepinsight/insightface/releases/download/v0.7/buffalo_l.zip" `
        -OutFile $zipM
    Expand-Archive -Path $zipM -DestinationPath "$MODELS_DIR\buffalo_l_tmp" -Force
    Remove-Item $zipM -Force
    $arc = Get-ChildItem "$MODELS_DIR\buffalo_l_tmp" -Filter "w600k_r50.onnx" -Recurse | Select-Object -First 1
    if ($arc) { Copy-Item $arc.FullName $ARCFACE -Force }
    Remove-Item "$MODELS_DIR\buffalo_l_tmp" -Recurse -Force
}
if (Test-Path $ARCFACE) { OK "arcface.onnx" } else { Warn "arcface.onnx missing - face recognition disabled" }

# Haar cascade fallback (copy from OpenCV installation)
Info "Checking Haar cascade (fallback)..."
if (-not (Test-Path $CASCADE)) {
    $src = "$OPENCV_BUILD\etc\haarcascades\haarcascade_frontalface_default.xml"
    if (Test-Path $src) { Copy-Item $src $CASCADE -Force; OK "haarcascade copied" }
    else { Warn "haarcascade not found - app will use full frame if YuNet also fails" }
}

# --- Build ---
$EXE_RELEASE = "$BUILD\Release\facecontrol.exe"
$EXE_FLAT    = "$BUILD\facecontrol.exe"
$needBuild   = $Rebuild -or (-not (Test-Path $EXE_RELEASE) -and -not (Test-Path $EXE_FLAT))

if ($needBuild) {
    Info "Configuring CMake..."
    New-Item -ItemType Directory -Force -Path $BUILD | Out-Null

    & $CMAKE_EXE `
        -S $ROOT -B $BUILD `
        "-DCMAKE_PREFIX_PATH=$QT_DIR" `
        "-DOpenCV_DIR=$OPENCV_CMAKE" `
        -DCMAKE_BUILD_TYPE=Release `
        -GNinja

    if ($LASTEXITCODE -ne 0) { Fatal "CMake configuration failed." }

    Info "Building (2-5 minutes)..."
    & $CMAKE_EXE --build $BUILD --config Release -j4
    if ($LASTEXITCODE -ne 0) { Fatal "Build failed." }

    $exePath = if (Test-Path $EXE_RELEASE) { $EXE_RELEASE } else { $EXE_FLAT }
    $exeDir  = Split-Path $exePath -Parent

    Info "Deploying Qt DLLs..."
    & $WINDEPLOY --release --no-translations $exePath

    # Copy OpenCV DLL
    foreach ($vcDir in @("vc17", "vc16", "vc15")) {
        $dlls = Get-ChildItem "$OPENCV_BUILD\x64\$vcDir\bin" -Filter "opencv_world*.dll" -ErrorAction SilentlyContinue |
                Where-Object { $_.Name -notlike "*d.dll" }
        if ($dlls) {
            foreach ($d in $dlls) { Copy-Item $d.FullName $exeDir -Force }
            OK "OpenCV DLL copied"
            break
        }
    }

    # Copy models
    $modelsTarget = "$exeDir\models"
    New-Item -ItemType Directory -Force -Path $modelsTarget | Out-Null
    if (Test-Path $YUNET)   { Copy-Item $YUNET   $modelsTarget -Force }
    if (Test-Path $ARCFACE) { Copy-Item $ARCFACE $modelsTarget -Force }
    if (Test-Path $CASCADE) { Copy-Item $CASCADE $modelsTarget -Force }

    OK "Build complete!"
} else {
    OK "Already built. Use -Rebuild to rebuild."
}

# --- Run ---
$exePath = if (Test-Path $EXE_RELEASE) { $EXE_RELEASE } else { $EXE_FLAT }
if (-not (Test-Path $exePath)) { Fatal "Executable not found: $exePath" }

Write-Host ""
Write-Host "==========================================" -ForegroundColor Green
Write-Host "   Starting FaceControl...                " -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green
Write-Host ""
Write-Host "  Default admin password: admin" -ForegroundColor White
Write-Host ""

Start-Process $exePath
Read-Host "Press Enter to exit"
