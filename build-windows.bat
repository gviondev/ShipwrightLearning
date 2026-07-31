@echo off
setlocal EnableExtensions

REM Always work from the directory containing this file so double-clicking is safe.
pushd "%~dp0"
if errorlevel 1 (
    echo ERROR: Could not open the project directory.
    set "EXIT_CODE=1"
    goto :finish
)
set "PROJECT_DIR_READY=1"

set "BUILD_DIR=build\x64"
set "GENERATOR=Visual Studio 17 2022"
set "TOOLSET=v143"
set "ARCH=x64"

REM Double-clicking builds Debug. From a terminal, pass Debug or Release explicitly.
set "CONFIG=%~1"
if not defined CONFIG set "CONFIG=Debug"
if /I "%CONFIG%"=="Debug" set "CONFIG=Debug"
if /I "%CONFIG%"=="Release" set "CONFIG=Release"
if /I not "%CONFIG%"=="Debug" if /I not "%CONFIG%"=="Release" (
    echo ERROR: Unsupported build configuration "%CONFIG%".
    echo Usage: %~nx0 [Debug^|Release]
    set "EXIT_CODE=2"
    goto :finish
)

REM Prefer CMake from PATH, then fall back to its standard Windows location.
set "CMAKE_EXE="
where cmake.exe >nul 2>&1
if not errorlevel 1 set "CMAKE_EXE=cmake.exe"
if not defined CMAKE_EXE if exist "%ProgramFiles%\CMake\bin\cmake.exe" (
    set "CMAKE_EXE=%ProgramFiles%\CMake\bin\cmake.exe"
)
if not defined CMAKE_EXE (
    echo ERROR: CMake was not found.
    echo Install CMake 3.26 or newer, or add cmake.exe to PATH.
    set "EXIT_CODE=3"
    goto :finish
)

echo ============================================================
echo  Ship of Harkinian - %CONFIG% build
echo ============================================================
echo.

echo [1/3] Configuring the Visual Studio project...
"%CMAKE_EXE%" -S . -B "%BUILD_DIR%" -G "%GENERATOR%" -T "%TOOLSET%" -A "%ARCH%"
if errorlevel 1 (
    set "EXIT_CODE=%errorlevel%"
    goto :build_failed
)

echo.
echo [2/3] Generating soh.o2r...
"%CMAKE_EXE%" --build "%BUILD_DIR%" --target GenerateSohOtr --config "%CONFIG%"
if errorlevel 1 (
    set "EXIT_CODE=%errorlevel%"
    goto :build_failed
)

echo.
echo [3/3] Building Ship of Harkinian...
"%CMAKE_EXE%" --build "%BUILD_DIR%" --config "%CONFIG%"
if errorlevel 1 (
    set "EXIT_CODE=%errorlevel%"
    goto :build_failed
)

set "EXIT_CODE=0"
echo.
echo ============================================================
echo  Build complete
echo  Executable: x64\%CONFIG%\soh.exe
echo  Solution:   %BUILD_DIR%\Ship.sln
echo ============================================================
goto :finish

:build_failed
echo.
echo ============================================================
echo  Build failed with exit code %EXIT_CODE%.
echo ============================================================

:finish
if defined PROJECT_DIR_READY popd
echo.
echo Press any key to close this window...
pause >nul
exit /b %EXIT_CODE%
