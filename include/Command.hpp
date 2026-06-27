#pragma once

#include "VWrappUtils.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>
namespace ce {

    class CommandPool {
      public:
        explicit CommandPool(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface)
            : physicalDevice(physicalDevice), logicalDevice(logicalDevice), surface(surface) {

            // Get inidices of queue families from device
            ce::QueueFamilyIndices queueFamilyIndices = GetQueueFamilies(physicalDevice, this->surface);

            const VkCommandPoolCreateInfo poolInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = static_cast<uint32_t>(
                    queueFamilyIndices.graphicsFamily) // Queue Family type that buffers from this command pool will use
            };

            // Create a Graphics Queue Family Command Pool
            if (vkCreateCommandPool(this->logicalDevice, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create Commnad Pool");
            }
        }

        virtual ~CommandPool() {
            //
            vkDestroyCommandPool(logicalDevice, this->commandPool, nullptr);
        }

        VkCommandPool& getCommandPool() { return this->commandPool; }

      private:
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
        VkCommandPool commandPool; // graphicsCommandPool;
        VkSurfaceKHR surface;
    };
} // namespace ce
