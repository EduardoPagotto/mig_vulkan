#pragma once

#include "VWrappUtils.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>
namespace ce {

    class CommandPool {
      public:
        explicit CommandPool(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface)
            : logicalDevice(logicalDevice), surface(surface) {

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
        // VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
        VkCommandPool commandPool; // graphicsCommandPool;
        VkSurfaceKHR surface;
    };

    class CommandBuffer {

        explicit CommandBuffer(VkDevice device, VkCommandPool commandPool, size_t count) : device(device), commandPool(commandPool) {

            this->commandBuffers.resize(count);

            const VkCommandBufferAllocateInfo cbAllocInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, // VK_COMMAND_BUFFER_LEVEL_PRIMARY : Buffer you submit directly
                                                          // to queue. Can't be called by other buffers.
                                                          // VK_COMMAND_BUFFER_LEVEL_SECUNDARY : Buffer can't be called
                                                          // directly. cam be called from other buffe via
                                                          // "VkCmdExecuteCommand" when recording commands in primary buf
                .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())};

            // Allocate command buffers and place handles in array of buffers
            if (vkAllocateCommandBuffers(device, &cbAllocInfo, this->commandBuffers.data()) != VK_SUCCESS) {
                throw std::runtime_error("Failed to Allocate Command buffers!");
            }
        }

      private:
        void beginCommandBuffer(size_t index, VkCommandBufferUsageFlagBits flag) {

            // Information to begin the command buffer record
            const VkCommandBufferBeginInfo beginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = flag // We're only using the command buffer once, so set up for one time submit
            };

            // Begin recording transfer commands
            if (vkBeginCommandBuffer(commandBuffers[index], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("Failed to start recording a Command Buffer!");
            }
        }

        void endCommandBuffer(size_t index) {
            // End commands
            if (vkEndCommandBuffer(this->commandBuffers[index]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to stop recording a Command Buffer!");
            }
        }

        void submit(VkQueue queue) {
            // Queue submission information
            const VkSubmitInfo submitInfo{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,                                   //
                .commandBufferCount = static_cast<uint32_t>(this->commandBuffers.size()), //
                .pCommandBuffers = this->commandBuffers.data()                            //
            };

            // Submit transfer command to transfer queue and wait until it finishes
            if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("Failed to submit Command Buffer to Queue!");
            }

            vkQueueWaitIdle(queue);

            // Free temporary command buffer back to pool
            vkFreeCommandBuffers(this->device, this->commandPool, this->commandBuffers.size(), this->commandBuffers.data());
        }

        VkDevice device;
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;
    };

} // namespace ce
