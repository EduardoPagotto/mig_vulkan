cmake_minimum_required(VERSION 3.31)
project(VulkanTest)

set(CMAKE_CXX_FLAGS "-std=c++20 ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs -fstandalone-debug")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(SET_GLFW_ENABLE OFF)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_COLOR_MAKEFILE ON)
set(ENV{CLICOLOR_FORCE} 1)

add_subdirectory(src/cevk)

if(SET_GLFW_ENABLE)
add_definitions(-DSET_GLFW_ENABLE)
add_subdirectory(apps/glfw)
else(SET_GLFW_ENABLE)
add_subdirectory(apps/sdl3)
endif(SET_GLFW_ENABLE)



# clean
add_custom_target (clean-all COMMAND rm -rf ${CMAKE_BINARY_DIR}/*
                             COMMAND rm -rf ${CMAKE_BINARY_DIR}/.cmake
                             COMMAND rm -rf ${CMAKE_BINARY_DIR}/.ninja_*
                             COMMAND rm -rf ${CMAKE_SOURCE_DIR}/bin/*)
