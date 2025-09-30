```bat
@echo off
setlocal

REM ===== CONFIGURACIÓN =====
set PROJECT=src\d3d12_proxy.vcxproj
set CONFIG=Release
set PLATFORM=x64

echo ====================================
echo   BUILD CON MSVC v143 (Visual Studio 2022)
echo ====================================

REM ===== Inicializa entorno VS2022 =====
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
)

REM ===== Compilar =====
msbuild "%PROJECT%" /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /p:PlatformToolset=v143
if errorlevel 1 (
    echo.
    echo ❌ Error al compilar el proyecto
    pause
    exit /b 1
)

REM ===== Copiar DLL si la compilación fue exitosa =====
if exist "src\bin\Release\x64\d3d12.dll" (
    echo.
    echo ✅ Compilación exitosa
    if not exist "output" mkdir output
    copy /Y "src\bin\Release\x64\d3d12.dll" "output\d3d12.dll"
    echo DLL generado en: output\d3d12.dll
) else (
    echo.
    echo ❌ No se encontró d3d12.dll compilado
)

pause
```
