#include "buffers/CommandBuffer.hpp"
#include <stdexcept>

namespace ce {

    CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool commandPool, size_t count) : device(device), commandPool(commandPool) {

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

    CommandBuffer::~CommandBuffer() {
        // Free temporary command buffer back to pool
        vkFreeCommandBuffers(this->device, this->commandPool, static_cast<uint32_t>(this->commandBuffers.size()),
                             this->commandBuffers.data());
    }

    void CommandBuffer::clean(size_t index) {
        if (vkResetCommandBuffer(this->commandBuffers[index], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS) {
            throw std::runtime_error("Failed to reset a Command Buffer!");
        }
    }

    void CommandBuffer::cleanAll() {
        for (auto& commandBuffer : this->commandBuffers) {
            if (vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS) {
                throw std::runtime_error("Failed to reset a Command Buffer!");
            }
        }
    }

    void CommandBuffer::begin(size_t index, VkCommandBufferUsageFlagBits flag) {

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

    void CommandBuffer::end(size_t index) {
        // End commands
        if (vkEndCommandBuffer(this->commandBuffers[index]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to end a Command Buffer!");
        }
    }

} // namespace ce
