PROJECT(sdl_test VERSION 0.0.1 LANGUAGES CXX)

find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(SDL3 REQUIRED)

add_executable(${PROJECT_NAME} VulkanRenderer.cpp
                               main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        SDL3::SDL3
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
)
