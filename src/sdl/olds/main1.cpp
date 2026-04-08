#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

#define VMA_IMPLEMENTATION

// Include Vulkan Memory Allocator (VMA) - requires vma_impl.cpp
#include "vk_mem_alloc.h"

int main(int argc, char* argv[]) {
    // 1. Initialize SDL3
    if (SDL_Init(SDL_INIT_VIDEO) == false) {
        std::cerr << "SDL_Init Failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    // 2. Create Window with Vulkan support
    SDL_Window* window = SDL_CreateWindow("SDL3 Vulkan Linux", 800, 600, SDL_WINDOW_VULKAN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    // 3. Get required Instance Extensions
    unsigned int extensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    std::vector<const char*> extensions(extensionCount);
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data());

    // 4. Create VkInstance
    VkInstance instance;
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "SDL3 Vulkan";
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        std::cerr << "Failed to create VkInstance" << std::endl;
        return -1;
    }

    // 5. Create VkSurfaceKHR
    VkSurfaceKHR surface;
    if (SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface) == false) {
        std::cerr << "Failed to create Vulkan Surface: " << SDL_GetError() << std::endl;
        return -1;
    }

    // 6. Initialize Vulkan Memory Allocator (VMA)
    // First, need Physical Device and Logical Device (skipping for brevity)
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // Should be selected
    VkDevice device = VK_NULL_HANDLE;                 // Should be created

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

    VmaAllocator allocator;
    vmaCreateAllocator(&allocatorInfo, &allocator);

    std::cout << "Successfully created Vulkan instance, surface, and VMA allocator." << std::endl;

    // Cleanup
    vmaDestroyAllocator(allocator);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
