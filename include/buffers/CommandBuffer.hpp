#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

namespace ce {

    class CommandBuffer {
      public:
        explicit CommandBuffer(VkDevice device, VkCommandPool commandPool, size_t count);
        virtual ~CommandBuffer();

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        void clean(size_t index);
        void cleanAll();
        void begin(size_t index, VkCommandBufferUsageFlagBits flag);
        void end(size_t index);

        std::vector<VkCommandBuffer>& getBuffers() { return this->commandBuffers; }

      private:
        VkDevice device;
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;
    };

} // namespace ce
