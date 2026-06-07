#pragma once

#include "Device.hpp"
#include <memory>

namespace ce {

    struct SwapchainImage {
        VkImage image;
        VkImageView imageView;
    };

    class SwapChain {
      public:
        explicit SwapChain(std::shared_ptr<Device> dev);
        virtual ~SwapChain();

        VkSwapchainKHR& getSwapchain() { return swapchain; }
        VkExtent2D& getSwapchainExtent() { return swapchainExtent; }
        std::vector<SwapchainImage>& getSwapchainImages() { return swapchainImages; }
        VkFormat& getSwapchainImageFormat() { return swapchainImageFormat; }
        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;

      private:
        VkSwapchainKHR swapchain;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;
        std::vector<SwapchainImage> swapchainImages;

        std::shared_ptr<Device> dev;

        static VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes);
    };
} // namespace ce
