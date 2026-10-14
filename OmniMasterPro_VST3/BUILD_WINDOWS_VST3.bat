@echo off
setlocal EnableDelayedExpansion
title OmniMaster Pro - Windows 11 VST3 Automated Builder
color 0B
cd /d "%~dp0"

echo ============================================================================
echo   OMNIMASTER PRO - WINDOWS 11 VST3 BUILDER FOR FENDER STUDIO PRO
echo   Working Folder: %CD%
echo ============================================================================
echo.

:: 1. Locate or Install CMake
set "CMAKE_CMD=cmake"
where cmake >nul 2>nul
if %ERRORLEVEL% EQU 0 goto CHECK_GIT
if exist "C:\Program Files\CMake\bin\cmake.exe" set "CMAKE_CMD=C:\Program Files\CMake\bin\cmake.exe"
if exist "C:\Program Files\CMake\bin\cmake.exe" goto CHECK_GIT

echo [!] CMake is not installed on your PC yet.
echo      Installing CMake automatically via Windows 11 Winget...
winget install -e --id Kitware.CMake --accept-package-agreements --accept-source-agreements
if exist "C:\Program Files\CMake\bin\cmake.exe" set "CMAKE_CMD=C:\Program Files\CMake\bin\cmake.exe"

:CHECK_GIT
where git >nul 2>nul
if %ERRORLEVEL% EQU 0 goto CHECK_MSVC
if exist "C:\Program Files\Git\cmd\git.exe" set "PATH=%PATH%;C:\Program Files\Git\cmd"
where git >nul 2>nul
if %ERRORLEVEL% EQU 0 goto CHECK_MSVC

echo [!] Git is not installed on your PC yet (needed to download JUCE 8).
echo     Installing Git automatically via Windows 11 Winget...
winget install -e --id Git.Git --accept-package-agreements --accept-source-agreements
if exist "C:\Program Files\Git\cmd\git.exe" set "PATH=%PATH%;C:\Program Files\Git\cmd"

:CHECK_MSVC
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" goto RUN_BUILD
echo.
echo [!] Microsoft Visual C++ Build Tools were not detected on your PC.
echo     To compile a native 64-bit .vst3 plugin on Windows 11, Windows needs
echo     the free Microsoft Visual Studio C++ Build Tools.
echo.
set /p INSTALL_VS="Would you like to install Visual Studio C++ Build Tools automatically now? (Y/N): "
if /I "%INSTALL_VS%"=="Y" goto INSTALL_MSVC
goto SHOW_GITHUB_ALT

:INSTALL_MSVC
echo Installing Microsoft Visual Studio 2022 C++ Build Tools (this takes a few minutes)...
winget install -e --id Microsoft.VisualStudio.2022.BuildTools --override "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended" --accept-package-agreements --accept-source-agreements

:RUN_BUILD
echo.
echo [1/3] Configuring CMake and downloading JUCE 8 Audio Engine...
"%CMAKE_CMD%" -B build -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% NEQ 0 goto BUILD_ERROR

echo.
echo [2/3] Compiling 64-bit OmniMaster Pro.vst3 for Windows 11...
"%CMAKE_CMD%" --build build --config Release --parallel 4
if %ERRORLEVEL% NEQ 0 goto BUILD_ERROR

echo.
echo [3/3] Build Succeeded!
set "VST3_OUT=build\OmniMasterPro_artefacts\Release\VST3\OmniMaster Pro.vst3"
echo Compiled VST3 location: %CD%\%VST3_OUT%
echo.
echo Copying OmniMaster Pro.vst3 to C:\Program Files\Common Files\VST3\ ...
xcopy /E /I /Y "%VST3_OUT%" "C:\Program Files\Common Files\VST3\OmniMaster Pro.vst3\"
if %ERRORLEVEL% NEQ 0 goto MANUAL_COPY_MSG
echo.
echo [SUCCESS] OmniMaster Pro.vst3 is now installed in C:\Program Files\Common Files\VST3\!
echo Open Fender Studio Pro and drag OmniMaster Pro onto your Master Bus!
goto END_STAY_OPEN

:MANUAL_COPY_MSG
echo.
echo [NOTE] Windows requires Administrator permission to copy into C:\Program Files\Common Files\VST3.
echo Please copy this folder:
echo   %CD%\%VST3_OUT%
echo And paste it into:
echo   C:\Program Files\Common Files\VST3\
goto END_STAY_OPEN

:SHOW_GITHUB_ALT
echo.
echo ============================================================================
echo   NO-INSTALL CLOUD BUILD OPTION (GITHUB ACTIONS)
echo ============================================================================
echo If you do not want to install C++ compilers on your PC:
echo 1. Go to https://github.com/new and create a free repository.
echo 2. Drag all files from F:\OmniMasterPro_VST3_Windows11\OmniMasterPro_VST3 into it.
echo 3. Click the 'Actions' tab on GitHub and download the compiled .vst3 in 3 minutes!
goto END_STAY_OPEN

:BUILD_ERROR
echo.
echo [ERROR] The build encountered an error above.
echo Make sure CMake, Git, and Visual Studio Desktop C++ Build Tools are installed,
echo or use the GitHub Actions cloud build described in README.md.

:END_STAY_OPEN
echo.
pause
cmd /k