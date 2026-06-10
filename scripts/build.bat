@echo off
echo Building High-Performance Collatz Analyzer...

if not exist build (
    mkdir build
)

echo Compiling Main Application...
g++ src\main.cpp -o build\collatz.exe -O3 -I src

echo Compiling Test Suite...
g++ tests\test_collatz.cpp -o build\collatz_test.exe -O3 -I src

echo Build Complete.
echo Executable: build\collatz.exe
echo Test Suite: build\collatz_test.exe
