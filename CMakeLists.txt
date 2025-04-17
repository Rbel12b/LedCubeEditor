cmake_minimum_required(VERSION 3.15)
project(ledcubeeditor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

add_executable(ledcubeeditor ${SRC_FILES})

target_compile_options(ledcubeeditor PUBLIC -O3)

target_include_directories(ledcubeeditor PUBLIC src)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

target_link_libraries(ledcubeeditor
    imgui
    OpenGL::GL
    glfw
    glad
    glm::glm
    tinyfiledialogs
)