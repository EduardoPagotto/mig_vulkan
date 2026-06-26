#pragma once

#include "ImageObject.hpp"
#include "VWrapp.hpp"
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

        VkSwapchainKHR& getSwapchain() { return this->swapchain; }
        VkExtent2D& getSwapchainExtent() { return this->swapchainExtent; }
        std::vector<std::shared_ptr<ImageObject>>& getSwapchainImages() { return this->swapchainImages; }
        VkFormat& getSwapchainImageFormat() { return this->swapchainImageFormat; }

      private:
        VkSwapchainKHR swapchain;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;
        std::vector<std::shared_ptr<ImageObject>> swapchainImages;
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
