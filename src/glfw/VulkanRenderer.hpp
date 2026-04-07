#pragma once

#include <SDL3/SDL_vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>

class VulkanRenderer {
  public:
    VulkanRenderer();
    virtual ~VulkanRenderer();

    int init(GLFWwindow* window);

  private:
    GLFWwindow* window;

    // vulkan components
    VkInstance instance;

    // Vulkan Functions
    void createInstance();
};
