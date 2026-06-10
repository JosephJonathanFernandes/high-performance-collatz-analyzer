cmake_minimum_required(VERSION 3.10)
project(CollatzAnalyzer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable maximum optimizations for the compiler
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /W3")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra")
endif()

# Main Application Binary
add_executable(collatz src/main.cpp)
target_include_directories(collatz PRIVATE src)

# Test Binary
add_executable(collatz_test tests/test_collatz.cpp)
target_include_directories(collatz_test PRIVATE src)
