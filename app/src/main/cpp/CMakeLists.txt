cmake_minimum_required(VERSION 3.6)

project(Opengl)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${CMAKE_PROJECT_NAME} SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Application/App.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/GUI/ImguiComponent.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/GUI/SceneControlMenu.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Graphics/Shader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Graphics/Camera.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Scene/BasicScene.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui/imgui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui/imgui_demo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui/imgui_draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui/imgui_tables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui/imgui_widgets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui/backends/imgui_impl_android.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui/backends/imgui_impl_opengl3.cpp
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
  ${CMAKE_CURRENT_SOURCE_DIR}/Application
  ${CMAKE_CURRENT_SOURCE_DIR}/GUI
  ${CMAKE_CURRENT_SOURCE_DIR}/Scene
  ${CMAKE_CURRENT_SOURCE_DIR}/Graphics
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui
  ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui/backends
  ${CMAKE_CURRENT_SOURCE_DIR}/../../glm
  ${ANDROID_NDK}/sources/android/native_app_glue
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
  android
  EGL
  GLESv3
  log
)
