cmake_minimum_required(VERSION 3.6)
if (WIN32) 
  # 设置目标编译平台
  set(CMAKE_SYSTEM_NAME Android)
  
  # 设置目标系统版本
  set(CMAKE_SYSTEM_VERSION 33)
  
  # 设置 Android 平台版本
  set(ANDROID_PLATFORM 33)
  
  # 设置目标架构
  set(ANDROID_ABI arm64-v8a)
  
  # 设置NDK路径
  set(ANDROID_NDK "C:/Users/DDD/AppData/Local/Android/Sdk/ndk/25.2.9519653")
  
  # 设置交叉编译工具链的 CMake 配置文件
  set(CMAKE_TOOLCHAIN_FILE "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
endif()
project(ImGuiExample)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${CMAKE_PROJECT_NAME} SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui_demo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui_draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui_tables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui_widgets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../backends/imgui_impl_android.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../backends/imgui_impl_opengl3.cpp
  ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

set(CMAKE_SHARED_LINKER_FLAGS
  "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate"
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
  IMGUI_IMPL_OPENGL_ES3
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${CMAKE_CURRENT_SOURCE_DIR}/../../backends
  ${ANDROID_NDK}/sources/android/native_app_glue
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
  android
  EGL
  GLESv3
  log
)
