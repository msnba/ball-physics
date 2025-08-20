cmake_minimum_required(VERSION 3.10.0)
project(BallPhysics VERSION 0.1.0 LANGUAGES C CXX)

find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

set(REQUIRED_FILES
    main.cpp
    shader.hpp
)

set(DEPENDENCIES
    GLEW::GLEW
    glfw
    glm::glm
    glad::glad
)

add_executable(BallPhysics ${REQUIRED_FILES})

target_link_libraries(BallPhysics PRIVATE ${DEPENDENCIES})

# Any Shader with .vert or .frag will be copied to build folder at runtime

file(GLOB SHADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/*.frag
)

add_custom_command(
    TARGET BallPhysics POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SHADERS}
        $<TARGET_FILE_DIR:BallPhysics>
)