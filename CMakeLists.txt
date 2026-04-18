cmake_minimum_required(VERSION 3.31)
project(VulkanTest)

set(CMAKE_CXX_FLAGS "-std=c++20 ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs -fstandalone-debug")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(simples/sdl3)
add_subdirectory(simples/glfw)

# clean
add_custom_target (clean-all COMMAND rm -rf ${CMAKE_BINARY_DIR}/*
                             COMMAND rm -rf ${CMAKE_BINARY_DIR}/.cmake
                             COMMAND rm -rf ${CMAKE_BINARY_DIR}/.ninja_*
                             COMMAND rm -rf ${CMAKE_SOURCE_DIR}/bin/*)
