cmake_minimum_required(VERSION 3.31)

set(SET_GLFW_ENABLE ON)
add_definitions(-DSET_GLFW_ENABLE)

PROJECT(cevk VERSION 0.0.1 LANGUAGES CXX)

# set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)

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


set (IncludeDirs ${CMAKE_SOURCE_DIR}/cevk/include)

set(SourceFiles  ${CMAKE_SOURCE_DIR}/cevk/src/DevVK.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/debug.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/descriptors/DescriptorSet.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/descriptors/DescriptorSetLayout.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/descriptors/DescriptorPool.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/buffers/BufferObject.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/buffers/CommandBuffer.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/buffers/ImageObject.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/buffers/VBO.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/buffers/IBO.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/entitys/Textures.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/entitys/MeshModel.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/entitys/Mesh.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/CommandPool.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/SwapChain.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/Renderer.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/ShaderModule.cpp
                            ${CMAKE_SOURCE_DIR}/cevk/src/Pipeline.cpp)

add_library(${PROJECT_NAME} STATIC ${SourceFiles})

target_include_directories (${PROJECT_NAME} PRIVATE ${IncludeDirs})
