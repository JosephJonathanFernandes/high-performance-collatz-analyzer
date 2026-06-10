@echo off
echo Running Unit Tests...

if not exist build\collatz_test.exe (
    echo [ERROR] Test binary not found. Please run scripts\build.bat first.
    exit /b 1
)

.\build\collatz_test.exe
