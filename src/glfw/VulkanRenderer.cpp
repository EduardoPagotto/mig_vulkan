#include "VulkanRenderer.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

VulkanRenderer::VulkanRenderer() {}

VulkanRenderer::~VulkanRenderer() {}

int VulkanRenderer::init(GLFWwindow* window) {
    this->window = window;

    try {
        createInstance();
    } catch (const std::runtime_error& e) {
        printf("Error: %s\n", e.what());
        return EXIT_FAILURE;
    }

    return 0;
}

void VulkanRenderer::createInstance() {

    // Information about the aplication itself
    // Most data here doesn't affect program and is for developer convinience
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan app Teste";         // Custom name of the aplication
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Version app
    appInfo.pEngineName = "No engine";                     // Engine name
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);      // engine version
    appInfo.apiVersion = VK_API_VERSION_1_0;               // the version of vulkan

    // Create information for a VkInstance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    // createInfo.pNext = nullptr;
    // createInfo.flags = VK_WHATEVER | WHAT_EVER2
    createInfo.pApplicationInfo = &appInfo;

    // Create a List to hold instance extencios
    std::vector<const char*> instanceExtentions = std::vector<const char*>();

    // set up extentions will use
    uint32_t glfwExtentionCount = 0; // GLFW may require multiple extentions
    const char** glfwExtentions;     // Extentions passed as array of cstring,
    ;                                // so need pointer (the array) to pointer(the string)
    // Get glfw extentions
    glfwExtentions = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);

    // Add glwf extentions to list of extentions
    for (size_t i = 0; i < glfwExtentionCount; i++) {
        instanceExtentions.push_back(glfwExtentions[i]);
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtentions.size());
    createInfo.ppEnabledExtensionNames = instanceExtentions.data();

    // TODO: Set a validation layer tha instace will use
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;

    // Create instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan Instance");
    }
}
