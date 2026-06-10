#pragma once

#include "VWrapp.hpp"
#include <memory>

namespace ce {

    struct SwapchainImage {
        VkImage image;
        VkImageView imageView;
    };

    class SwapChain {
      public:
        explicit SwapChain(std::shared_ptr<VWrapp> vwrapp);
        virtual ~SwapChain();

        VkSwapchainKHR& getSwapchain() { return swapchain; }
        VkExtent2D& getSwapchainExtent() { return swapchainExtent; }
        std::vector<SwapchainImage>& getSwapchainImages() { return swapchainImages; }
        VkFormat& getSwapchainImageFormat() { return swapchainImageFormat; }

        static VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

      private:
        VkSwapchainKHR swapchain;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;
        std::vector<SwapchainImage> swapchainImages;

        std::shared_ptr<VWrapp> vwrapp;

        static VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes);
    };
} // namespace ce
