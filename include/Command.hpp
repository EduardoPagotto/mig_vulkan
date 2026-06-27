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
                throw std::runtime_error("Failed to create Command Pool");
            }
        }

        virtual ~CommandPool() {
            //
            vkDestroyCommandPool(logicalDevice, this->commandPool, nullptr);
        }

        void cleanup() {
            //
            if (vkResetCommandPool(this->logicalDevice, this->commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS) {
                throw std::runtime_error("Failed to Reset Command Pool");
            }
        }

        VkCommandPool& getPool() { return this->commandPool; }

      private:
        VkDevice logicalDevice;
        VkCommandPool commandPool;
        VkSurfaceKHR surface;
    };

    class CommandBuffer {
      public:
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

        std::vector<VkCommandBuffer>& getBuffers() { return this->commandBuffers; }

        virtual ~CommandBuffer() {
            // Free temporary command buffer back to pool
            vkFreeCommandBuffers(this->device, this->commandPool, static_cast<uint32_t>(this->commandBuffers.size()),
                                 this->commandBuffers.data());
        }

        void clean(size_t index) {
            if (vkResetCommandBuffer(this->commandBuffers[index], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS) {
                throw std::runtime_error("Failed to reset a Command Buffer!");
            }
        }

        void cleanAll() {
            for (auto& commandBuffer : this->commandBuffers) {
                if (vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to reset a Command Buffer!");
                }
            }
        }

        void begin(size_t index, VkCommandBufferUsageFlagBits flag) {

            // Information to begin the command buffer record
            const VkCommandBufferBeginInfo beginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = flag // We're only using the command buffer once, so set up for one time submit
            };

            // Begin recording transfer commands
            if (vkBeginCommandBuffer(commandBuffers[index], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("Failed to begin a Command Buffer!");
            }
        }

        void end(size_t index) {
            // End commands
            if (vkEndCommandBuffer(this->commandBuffers[index]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to end a Command Buffer!");
            }
        }

      private:
        VkDevice device;
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;
    };

} // namespace ce
