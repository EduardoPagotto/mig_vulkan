#pragma once

#ifdef SET_GLFW_ENABLE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#else
#include <SDL3/SDL_vulkan.h>
#endif

#include "Ultilities.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

class VulkanRenderer {
  public:
    VulkanRenderer() = default;
    virtual ~VulkanRenderer() {}

#ifdef SET_GLFW_ENABLE
    int init(GLFWwindow* window) {
        this->window = window;
        return init_vulkan();
    }
#else
    int init(SDL_Window* window) {
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
    std::vector<VkFramebuffer> swapChainFrameBuffers;
    std::vector<VkCommandBuffer> commandBuffers;

    // - Pipeline
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;

    // - Pools
    VkCommandPool graphicsCommandPool;

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
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();

    // - Record Functions
    void recordCommand();

    // - Get Functions
    void getPhysicalDevice();

    // - Suport Functions
    // -- Checker functions
    bool static checkInstanceExtentionsSupport(std::vector<const char*>* checkExtentions);
    bool static checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool static checkValidationLayerSupport();
    bool checkDeviceSuitable(VkPhysicalDevice device);

    // -- Getter Functions
    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
    SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);

    // - Choose functions
    VkSurfaceFormatKHR static chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR static chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

    // -- Create Functions
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;
    [[nodiscard]] VkShaderModule createShaderModule(const std::vector<char>& code) const;

    // generic
    int init_vulkan();
};
