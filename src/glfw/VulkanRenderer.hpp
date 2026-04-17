#pragma once

#define GLFW_INCLUDE_VULKAN
#include "Ultilities.hpp"
#include <GLFW/glfw3.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

#include <stdexcept>
#include <vector>

class VulkanRenderer {
  public:
    VulkanRenderer();
    virtual ~VulkanRenderer();

    int init(GLFWwindow* window);
    void cleanup();

  private:
    GLFWwindow* window;

    // vulkan components
    VkInstance instance;
    struct {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } mainDevice;
    VkQueue graphicsQueue;
    VkQueue presentationQueue;

    //
    VkSurfaceKHR surface;

    // Vulkan Functions
    // - Create functions
    void createInstance();
    void createLogicalDevices();
    void createSurface();

    // - Get Functions
    void getPhysicalDevice();

    // - Suport Functions
    // -- Checker functions
    bool checkInstanceExtentionsSupport(std::vector<const char*>* checkExtentions);
    bool checkDeviceSuitable(VkPhysicalDevice device);

    // -- Getter Functions
    QueueFamilyIndices getQueueFamillies(VkPhysicalDevice device);
};
