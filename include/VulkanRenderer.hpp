#pragma once

#ifdef SET_GLFW_ENABLE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#else
#endif

#include "Ultilities.hpp"
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

// #include <stdexcept>
#include <vector>

class VulkanRenderer {
  public:
    VulkanRenderer();
    virtual ~VulkanRenderer();

#ifdef SET_GLFW_ENABLE
    int init(GLFWwindow* window) {
        this->window = window;
        return init_vulkan();
    }
#else
    bool init(SDL_Window* window) {
        this->window = window;
        return init_vulkan();
    }
#endif

    void cleanup();

  private:
#ifdef SET_GLFW_ENABLE
    GLFWwindow* window;
#else
    SDL_Window* window;
#endif

    // Vulkan components
    // - Main
    VkInstance instance;
    VkDebugReportCallbackEXT callback;
    struct {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } mainDevice;
    VkQueue graphicsQueue;
    VkQueue presentationQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    std::vector<SwapchainImage> swapchainImages;

    // - Utility
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;

    // Vulkan Functions
    // - Create functions
    void createInstance();
    void createDebugCallback();
    void createLogicalDevices();
    void createSurface();
    void createSwapChain();

    // - Get Functions
    void getPhysicalDevice();

    // - Suport Functions
    // -- Checker functions
    bool checkInstanceExtentionsSupport(std::vector<const char*>* checkExtentions);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool checkValidationLayerSupport();
    bool checkDeviceSuitable(VkPhysicalDevice device);

    // -- Getter Functions
    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
    SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);

    // - Choose functions
    VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

    // -- Create Functions
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    // generic
    int init_vulkan();
};
