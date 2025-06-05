cmake_minimum_required(VERSION 3.10)

project(TinyAArch64)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} Main.cpp CPU.cpp Assembler.cpp)