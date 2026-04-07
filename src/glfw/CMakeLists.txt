cmake_minimum_required(VERSION 3.31)

PROJECT(TutorialVulkan VERSION 0.0.1 LANGUAGES CXX)

set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)

find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

add_executable(${PROJECT_NAME} VulkanRenderer.cpp
                               main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw vulkan)
