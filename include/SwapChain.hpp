#pragma once

#include "VWrapp.hpp"
#include <memory>
#include <vector>

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

      private:
        VkSwapchainKHR swapchain;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;
        std::vector<SwapchainImage> swapchainImages;

        std::shared_ptr<VWrapp> vwrapp;
    };
} // namespace ce
