@echo off
echo.
echo ========================================================
echo      Master Mind Trading System - Windows Build
echo ========================================================
echo.

REM Check if CMake is available
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found in PATH
    echo Please install CMake and add it to your PATH
    echo Download from: https://cmake.org/download/
    pause
    exit /b 1
)

REM Check if Qt6 is available
if not exist "C:\Qt\6.8.3\msvc2022_64" (
    echo WARNING: Qt6 not found at expected location
    echo Please install Qt6.8.3 with MSVC2022 64-bit compiler
    echo Or update CMAKE_PREFIX_PATH in CMakeLists.txt
    echo.
    echo Continue anyway? (y/n)
    set /p continue=
    if /i not "%continue%"=="y" exit /b 1
)

REM Create build directory
if not exist build mkdir build
cd build

echo Configuring project...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: CMake configuration failed
    echo.
    echo Common solutions:
    echo 1. Install Qt6.8.3 with MSVC2022 64-bit
    echo 2. Install OpenSSL for Windows
    echo 3. Install Visual Studio 2022 with C++ tools
    echo 4. Check CMAKE_PREFIX_PATH in CMakeLists.txt
    echo.
    pause
    exit /b 1
)

echo.
echo Building project...
cmake --build . --config Release --parallel

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed
    echo Check the error messages above
    echo.
    echo Common solutions:
    echo 1. Install missing dependencies
    echo 2. Check C++ compiler version (requires C++17)
    echo 3. Verify all source files are present
    echo.
    pause
    exit /b 1
)

echo.
echo Running tests...
ctest --config Release --verbose

echo.
echo ========================================================
echo Build completed successfully!
echo.
echo Executables created:
echo - Release\MasterMindTraderGUI.exe (GUI version)
echo - Release\MasterMindTrader.exe (Console version)
echo - Release\MasterMindTest.exe (Test suite)
echo.
echo To run the GUI version:
echo   cd build\Release
echo   MasterMindTraderGUI.exe
echo.
echo To run tests:
echo   cd build
echo   ctest --config Release
echo ========================================================
echo.

pause 