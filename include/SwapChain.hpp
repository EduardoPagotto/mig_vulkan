#pragma once

#include "ImageObject.hpp"
#include "VWrapp.hpp"
#include <memory>
#include <vector>

namespace ce {

    class SwapChain {
      public:
        explicit SwapChain(std::shared_ptr<VWrapp> vwrapp);
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

        std::shared_ptr<VWrapp> vwrapp;
    };
} // namespace ce
