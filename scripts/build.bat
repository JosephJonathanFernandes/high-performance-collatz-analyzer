@echo off
echo Building High-Performance Collatz Analyzer...

if not exist build (
    mkdir build
)
cd build

echo Configuring CMake...
cmake ..

echo Compiling Release Build...
cmake --build . --config Release

echo Build Complete. You can run the executable at build\Release\collatz.exe
cd ..
