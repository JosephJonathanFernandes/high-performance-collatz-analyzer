@echo off
echo Running Unit Tests...

if not exist build\Release\collatz_test.exe (
    echo [ERROR] Test binary not found. Please run scripts\build.bat first.
    exit /b 1
)

cd build
.\Release\collatz_test.exe
cd ..
