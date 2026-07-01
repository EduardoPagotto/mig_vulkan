#pragma once

#include "buffers/ImageObject.hpp"
#include "subsystem.hpp"
#include <memory>
#include <vector>

namespace ce {

    class SwapChain {
      public:
        explicit SwapChain(std::shared_ptr<BaseVK> bvk);
        virtual ~SwapChain();

        VkSwapchainKHR& getKHR() { return this->swapchain; }
        VkExtent2D& getExtent() { return this->extent; }
        std::vector<std::shared_ptr<ImageObject>>& getImages() { return this->images; }
        VkFormat& getImageFormat() { return this->imageFormat; }
        std::vector<VkFramebuffer>& getSwapChainFrameBuffers() { return this->swapChainFrameBuffers; }

        void createFramebuffers(VkImageView& imageView, VkRenderPass& renderPass);

      private:
        VkSwapchainKHR swapchain;
        VkFormat imageFormat;
        VkExtent2D extent;
        std::shared_ptr<BaseVK> bvk;
        std::vector<std::shared_ptr<ImageObject>> images;
        std::vector<VkFramebuffer> swapChainFrameBuffers;

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

        static VkSurfaceFormatKHR ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static VkPresentModeKHR ChooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes);
    };
} // namespace ce
