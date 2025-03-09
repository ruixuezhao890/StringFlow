cmake_minimum_required(VERSION 3.30)
project(test_lib)

set(CMAKE_CXX_STANDARD 17)
include_directories(${PROJECT_SOURCE_DIR}/StringFlow
    examples/
)
file(GLOB SOURCES "StringFlow/include/*.cpp" "examples/*.cpp")
add_executable(test_lib ${SOURCES} main.cpp)
