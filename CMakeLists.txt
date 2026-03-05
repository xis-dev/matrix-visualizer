cmake_minimum_required(VERSION 3.31)
project(gl_scene LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
include(FetchContent)


FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
)

FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.89.9
)
FetchContent_MakeAvailable(imgui)


set (LOCAL_DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/dependencies)

set(GLAD_DIR ${LOCAL_DEPENDENCIES_DIR}/glad)

set(GLM_DIR ${LOCAL_DEPENDENCIES_DIR}/glm)

set(ALL_DEPENDENCIES_DIRS
        ${LOCAL_DEPENDENCIES_DIR}
        ${GLAD_DIR}
        ${GLM_DIR}
        ${glfw_SOURCE_DIR}
        ${imgui_SOURCE_DIR})


set (INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/includes
)
set (HEADER_SOURCES
        includes/PathResolver.h)

set(PROJECT_SOURCES
        ${GLAD_DIR}/glad.c
        src/main.cpp
        src/Shader.cpp
        src/Camera.cpp )

set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)


add_executable(gl-scene ${PROJECT_SOURCES} ${IMGUI_SOURCES} ${HEADER_SOURCES} )


target_include_directories(gl-scene PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${INCLUDE_DIRS}
        ${ALL_DEPENDENCIES_DIRS}

        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(gl-scene PRIVATE
        glfw
        ${OPENGL_LIBRARIES}
)

