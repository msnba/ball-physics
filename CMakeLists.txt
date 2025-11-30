cmake_minimum_required(VERSION 3.15)
project(BallPhysics VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

set(SOURCES
    main.cpp
)

add_executable(BallPhysics ${SOURCES})

target_link_libraries(BallPhysics PRIVATE
    glfw
    glm::glm
    glad::glad
)

# Platform Specifics
if(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)

    target_link_libraries(BallPhysics PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${OPENGL_LIBRARY}
    )

    target_compile_definitions(BallPhysics PRIVATE "GL_SILENCE_DEPRECATION")
endif()