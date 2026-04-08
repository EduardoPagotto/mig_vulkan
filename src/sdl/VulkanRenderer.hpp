#pragma once

#include "Ultilities.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
#include <vulkan/vulkan_core.h>

class VulkanRenderer {
  public:
    VulkanRenderer();
    virtual ~VulkanRenderer();

    bool init(SDL_Window* window);
    void cleanup();

  private:
    SDL_Window* window;

    // vulkan components
    VkInstance instance;
    struct {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } mainDevice;
    VkQueue graphicsQueue;

    // Vulkan Functions
    // - Create functions
    void createInstance();
    void createLogicalDevices();

    // - Get Functions
    void getPhysicalDevice();

    // - Suport Functions
    // -- Checker functions
    bool checkInstanceExtentionsSupport(std::vector<const char*>* checkExtentions);
    bool checkDeviceSuitable(VkPhysicalDevice device);

    // -- Getter Functions
    QueueFamilyIndices getQueueFamillies(VkPhysicalDevice device);
};
