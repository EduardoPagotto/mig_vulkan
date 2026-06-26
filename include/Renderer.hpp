#pragma once
#include <vulkan/vulkan_core.h>

namespace ce {

    class Renderer {
      public:
        explicit Renderer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, const VkFormat& format);
        virtual ~Renderer();

        VkRenderPass& getRenderPass() { return renderPass; }

      private:
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;

        VkRenderPass renderPass;

        void createRenderPass(const VkFormat& format);
    };
} // namespace ce
