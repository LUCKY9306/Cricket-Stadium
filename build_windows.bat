@echo off
setlocal
REM ===========================================================================
REM  Build script for Windows + MinGW-w64 + freeglut
REM  Uses the local FreeGLUT install created from the source tree.
REM ===========================================================================

set "INCDIR=C:\freeglut\include"
set "LIBDIR=C:\freeglut\lib"
set "DLLDIR=C:\freeglut\bin"

if not exist "%INCDIR%\GL\freeglut.h" (
    echo FreeGLUT headers not found at %INCDIR%\GL
    echo Install FreeGLUT and make sure C:\freeglut is valid.
    pause
    exit /b 1
)

if not exist "%LIBDIR%\libfreeglut.dll.a" (
    echo FreeGLUT import library not found at %LIBDIR%
    echo Build FreeGLUT from the local source tree first.
    pause
    exit /b 1
)

echo Compiling 3D Cricket Stadium Simulation...

g++ -O2 -Wall -I"%INCDIR%" ^
    main.cpp camera.cpp stadium.cpp animation.cpp lighting.cpp texture.cpp input.cpp ^
    -o CricketStadium.exe ^
    -L"%LIBDIR%" -lfreeglut -lopengl32 -lglu32

if errorlevel 1 (
    echo.
    echo BUILD FAILED - check the MinGW and FreeGLUT install.
    pause
    exit /b 1
)

echo.
if exist "%DLLDIR%\libfreeglut.dll" (
    copy /Y "%DLLDIR%\libfreeglut.dll" . >nul
    echo DLL copied next to the executable.
) else (
    echo WARNING: libfreeglut.dll was not found in %DLLDIR%
)

echo Build OK. Run CricketStadium.exe.
pause
endlocal
