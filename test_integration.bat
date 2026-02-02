@echo off
echo ================================
echo MPlayer集成测试脚本
echo ================================

echo.
echo 1. 检查MPlayer可执行文件...
if exist "mplayer\debug\mplayer.exe" (
    echo [OK] 找到Debug版本MPlayer
) else if exist "mplayer\release\mplayer.exe" (
    echo [OK] 找到Release版本MPlayer
) else (
    echo [ERROR] 未找到MPlayer可执行文件！
    echo 请先编译mplayer项目
    pause
    exit /b 1
)

echo.
echo 2. 检查MultiMediaTool可执行文件...
if exist "MultiMediaTool\bin\MultiMediaTool.exe" (
    echo [OK] 找到MultiMediaTool
) else (
    echo [WARNING] 未找到MultiMediaTool，请先编译MultiMediaTool项目
)

echo.
echo 3. 启动测试...
echo 使用方法：
echo 1. 启动MultiMediaTool
echo 2. 进入视频页面
echo 3. 点击"开始"按钮
echo 4. 选择视频文件进行播放
echo.

if exist "MultiMediaTool\bin\MultiMediaTool.exe" (
    echo 启动MultiMediaTool...
    cd MultiMediaTool\bin
    start MultiMediaTool.exe
    echo MultiMediaTool已启动
) else (
    echo 请先编译项目，然后手动启动MultiMediaTool
)

echo.
echo ================================
echo 测试完成
echo ================================
pause