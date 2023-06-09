# Project settings
cmake_minimum_required(VERSION 3.11)
project("A* Pathfinding")

# Compiler settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Main executable
include_directories(${PROJECT_SOURCE_DIR})
add_executable(astar src/main.cpp src/shader.cpp)
file(COPY ${PROJECT_SOURCE_DIR}/src/vertex.glsl DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/src/fragment.glsl DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# GLFW
add_subdirectory(lib/glfw)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
target_link_libraries(astar glfw)

# GLAD
add_subdirectory(lib/glad)
target_link_libraries(astar glad)