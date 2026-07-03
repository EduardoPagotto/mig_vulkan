cmake_minimum_required(VERSION 3.31)

PROJECT(sdl_test VERSION 0.0.1 LANGUAGES CXX)

find_package(SDL3 REQUIRED)
find_package(assimp REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

set (IncludeDirs ${CMAKE_CURRENT_SOURCE_DIR}/include
                 ${CMAKE_SOURCE_DIR}/src/cevk/include/cevk)

set (LIBRARY_DEPS ${CMAKE_SOURCE_DIR}/bin/libcevk.a
                  SDL3::SDL3
                  Vulkan::Vulkan
                  GPUOpen::VulkanMemoryAllocator
                  assimp::assimp)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanRenderer.cpp
                               ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

target_include_directories (${PROJECT_NAME} PRIVATE ${IncludeDirs})

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARY_DEPS})

add_dependencies(${PROJECT_NAME} cevk)
