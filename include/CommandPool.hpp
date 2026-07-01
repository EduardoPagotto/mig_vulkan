#pragma once

#include <vulkan/vulkan_core.h>

namespace ce {

    class CommandPool {
      public:
        explicit CommandPool(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface);
        virtual ~CommandPool();
        void cleanup();
        VkCommandPool& getPool() { return this->commandPool; }

      private:
        VkDevice logicalDevice;
        VkCommandPool commandPool;
        VkSurfaceKHR surface;
    };
} // namespace ce
