#pragma once

#include <array>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ce {

    struct SubmitToRenderInfo {
        VkQueue gQueue;
        VkSemaphore wait;
        VkSemaphore signal;
        VkFence fence;
        VkPipelineStageFlagBits pipelineStageFlags;
        size_t bufferIndex;
    };

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

        void submitToRender(const SubmitToRenderInfo& sub) {
            // -- SUBMIT COMMAND BUFFER TO RENDER
            // Queue submission information
            std::array<VkSemaphore, 1> waitSemaphores{sub.wait};                    //{this->imageAvailable[this->currentFrame]};
            std::array<VkSemaphore, 1> signalSemaphores{sub.signal};                //{this->renderFinished[this->currentFrame]};
            std::array<VkPipelineStageFlags, 1> waitStages{sub.pipelineStageFlags}; //{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

            const VkSubmitInfo submitInfo{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()), // Number of semaphores to wait on
                .pWaitSemaphores = waitSemaphores.data(),                           //
                .pWaitDstStageMask = waitStages.data(),                             // Stagegs to check semaphores at
                .commandBufferCount = 1,                                   // Number of command buffers to submit FIXME: é isto mesmo?
                .pCommandBuffers = &this->commandBuffers[sub.bufferIndex], // Command buffer to submit
                .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()), // Number of semaphore to signal
                .pSignalSemaphores = signalSemaphores.data(),                           // Semaphore to signal when command buffer finishes
            };

            // Submit command buffer to queue
            if (vkQueueSubmit(sub.gQueue, 1, &submitInfo, sub.fence) != VK_SUCCESS) {
                throw std::runtime_error("Failed to submit Command Buffer to Queue!");
            }
        }

      private:
        VkDevice device;
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;
    };

} // namespace ce
