cmake_minimum_required(VERSION 3.26)

project(TinyAArch64)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-w)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
    ${CMAKE_SOURCE_DIR}/external/imgui/examples/libs/glfw/include
    # ${CMAKE_SOURCE_DIR}/external/fmt
)

link_directories(
    ${CMAKE_SOURCE_DIR}/external/imgui/examples/libs/glfw/lib-vc2010-64
)

set(SOURCES
    # ${CMAKE_SOURCE_DIR}/Test.cpp
    ${CMAKE_SOURCE_DIR}/Main.cpp
    ${CMAKE_SOURCE_DIR}/CPU.cpp
    ${CMAKE_SOURCE_DIR}/Assembler.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} glfw3 opengl32)
