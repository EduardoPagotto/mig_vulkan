PROJECT(sdl_test VERSION 0.0.1 LANGUAGES CXX)

find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(SDL3 REQUIRED)

set (IncludeDirs ${CMAKE_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/VulkanRenderer.cpp
                               main.cpp)

target_include_directories (${PROJECT_NAME} PRIVATE ${IncludeDirs})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        SDL3::SDL3
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
)
