#pragma once

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
namespace ce {
    class Sync {
      public:
        explicit Sync(VkDevice logical, size_t max) : logical(logical), max(max) {
            //
            this->imageAvailable.resize(max); // ce::MAX_FRAME_DRAWS
            this->renderFinished.resize(max);
            this->drawFences.resize(max);

            // Semaphore creation information
            const VkSemaphoreCreateInfo semaphoreCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            // Fence creation information
            const VkFenceCreateInfo fenceCreateInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};

            for (size_t i = 0; i < max; i++) {

                if (vkCreateSemaphore(logical, &semaphoreCreateInfo, nullptr, &this->imageAvailable[i]) != VK_SUCCESS ||
                    vkCreateSemaphore(logical, &semaphoreCreateInfo, nullptr, &this->renderFinished[i]) != VK_SUCCESS ||
                    vkCreateFence(logical, &fenceCreateInfo, nullptr, &this->drawFences[i]) != VK_SUCCESS) {

                    throw std::runtime_error("Failed to create a Semaphore and/or Fence!");
                }
            }
        }

        virtual ~Sync() {
            for (size_t i = 0; i < this->max; i++) {
                vkDestroySemaphore(this->logical, this->renderFinished[i], nullptr);
                vkDestroySemaphore(this->logical, this->imageAvailable[i], nullptr);
                vkDestroyFence(this->logical, this->drawFences[i], nullptr);
            }
        }

        void waitAndResetFence(size_t index) {
            // Wait for given fence to signal (open) from last draw before continuing
            vkWaitForFences(this->logical, 1, &this->drawFences[index], VK_TRUE, std::numeric_limits<uint64_t>::max());
            // Manually reset (close) fence
            vkResetFences(this->logical, 1, &this->drawFences[index]);
        }

        VkSemaphore& getWaitSemafore(size_t index) { return this->imageAvailable[index]; }
        VkSemaphore& getSignalSemaphore(size_t index) { return this->renderFinished[index]; }
        VkFence& getDrawFence(size_t index) { return this->drawFences[index]; }

      private:
        VkDevice logical;
        size_t max;
        // - Synchronization
        std::vector<VkSemaphore> imageAvailable;
        std::vector<VkSemaphore> renderFinished;
        std::vector<VkFence> drawFences;
    };
} // namespace ce
