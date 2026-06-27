@echo off
echo ========================================================
echo Attempting to compile with g++...
echo ========================================================

if not exist build mkdir build

echo Trying g++ with -std=c++17...
g++ -O3 -std=c++17 -I src src/main.cpp -o build/collatz.exe
if %ERRORLEVEL% equ 0 goto success

echo Trying g++ with -std=c++14...
g++ -O3 -std=c++14 -I src src/main.cpp -o build/collatz.exe
if %ERRORLEVEL% equ 0 goto success

echo Trying g++ with -std=c++11...
g++ -O3 -std=c++11 -I src src/main.cpp -o build/collatz.exe
if %ERRORLEVEL% equ 0 goto success

echo Trying g++ with default standard...
g++ -O3 -I src src/main.cpp -o build/collatz.exe
if %ERRORLEVEL% equ 0 goto success

echo.
echo [ERROR] Could not compile the C++ code with your version of g++.
exit /b 1

:success
echo.
echo ========================================================
echo Build Successful! Running Influence Analyzer...
echo ========================================================
.\build\collatz.exe influence
