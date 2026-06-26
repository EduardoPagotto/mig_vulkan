#pragma once

// FIXME: removendo builtins e usando apenas VK
#ifdef SET_GLFW_ENABLE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#else
#include <SDL3/SDL_vulkan.h>
#endif
#include "ImageObject.hpp"
#include <memory>
#include <vector>

namespace ce {

    class SwapChain {
      public:
#ifdef SET_GLFW_ENABLE
        explicit SwapChain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, GLFWwindow* window);
#else
        explicit SwapChain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, SDL_Window* window);
#endif

        virtual ~SwapChain();

        VkSwapchainKHR& getKHR() { return this->swapchain; }
        VkExtent2D& getExtent() { return this->extent; }
        std::vector<std::shared_ptr<ImageObject>>& getImages() { return this->images; }
        VkFormat& getImageFormat() { return this->imageFormat; }

      private:
        VkSwapchainKHR swapchain;
        VkFormat imageFormat;
        VkExtent2D extent;
        std::vector<std::shared_ptr<ImageObject>> images;
        VkDevice logicalDevice;

#ifdef SET_GLFW_ENABLE
        GLFWwindow* window;
#else
        SDL_Window* window;
#endif
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

        static VkSurfaceFormatKHR ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static VkPresentModeKHR ChooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes);
    };
} // namespace ce
