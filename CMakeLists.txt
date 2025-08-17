cmake_minimum_required(VERSION 3.10.0)
project(BallPhysics VERSION 0.1.0 LANGUAGES C CXX)

find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

set(REQUIRED_FILES
    main.cpp
)

set(DEPENDENCIES
    GLEW::GLEW
    glfw
    glm::glm
)

add_executable(BallPhysics ${REQUIRED_FILES})

target_link_libraries(BallPhysics PRIVATE ${DEPENDENCIES})