cmake_minimum_required(VERSION 3.31)

PROJECT(glwf_test VERSION 0.0.1 LANGUAGES CXX)

find_package(glfw3 3.3 REQUIRED)
find_package(assimp REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

# Compilar SpirV
# find_program(GLSLANG_VALIDATOR "glslangValidator")
# if(NOT GLSLANG_VALIDATOR)
#     message(FATAL_ERROR "glslangValidator não encontrado. Instale o Vulkan SDK.")
# endif()

# set(SHADER_SOURCES
#     ./shaders/shader.vert
#     ./shaders/shader.frag
# )

# set(SPV_OUTPUTS "./bin")


set (IncludeDirs ${CMAKE_SOURCE_DIR}/apps/include
                 ${CMAKE_SOURCE_DIR}/src/cevk/include/cevk)

set (LIBRARY_DEPS ${CMAKE_SOURCE_DIR}/bin/libcevk.a
                  glfw
                  vulkan
                  assimp::assimp)

add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/apps/src/VulkanRenderer.cpp
                               main.cpp)

target_include_directories (${PROJECT_NAME} PRIVATE ${IncludeDirs})

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARY_DEPS})

add_dependencies(${PROJECT_NAME} cevk)
