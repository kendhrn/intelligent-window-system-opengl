@echo off
setlocal EnableDelayedExpansion
cd /d "%~dp0"

rem Find a MinGW-w64 g++ that has freeGLUT installed.
rem Set CXX to a specific g++.exe to skip the search.
set "GXX="
set "CANDIDATES="
if defined CXX set "CANDIDATES="%CXX%""
for /f "delims=" %%G in ('where g++ 2^>nul') do set "CANDIDATES=!CANDIDATES! "%%G""
for %%D in (
    "C:\msys64\ucrt64\bin"
    "C:\msys64\mingw64\bin"
    "C:\mingw64\bin"
) do if exist "%%~D\g++.exe" set "CANDIDATES=!CANDIDATES! "%%~D\g++.exe""

for %%G in (!CANDIDATES!) do if not defined GXX call :check "%%~G"

if not defined GXX (
    echo Could not find a g++ with freeGLUT installed.
    echo Install MSYS2 and run:  pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-freeglut
    echo or set CXX to the full path of a suitable g++.exe.
    pause
    exit /b 1
)

for %%D in ("%GXX%") do set "PATH=%%~dpD;%PATH%"
echo Using %GXX%
echo Building iws.exe...
"%GXX%" -O2 -DFREEGLUT_STATIC main.cpp -o iws.exe -static -lfreeglut_static -lopengl32 -lglu32 -lwinmm -lgdi32
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

echo Build succeeded. Starting...
start "" iws.exe
exit /b 0

:check
rem A compiler qualifies if it can locate the static freeGLUT library
for /f "delims=" %%L in ('""%~1" -print-file-name=libfreeglut_static.a 2>nul"') do (
    if /i not "%%L"=="libfreeglut_static.a" set "GXX=%~1"
)
exit /b 0
