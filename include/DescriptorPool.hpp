#pragma once

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ce {

    class DescriptorPool {
      public:
        explicit DescriptorPool(VkDevice device) : device(device) {
            //
        }

        virtual ~DescriptorPool() { vkDestroyDescriptorPool(device, this->descriptorPool, nullptr); }

        void addPoolSize(VkDescriptorType type, uint32_t count) {

            // Type of Descriptors + how many DESCRIPTORS, not Descriptor Sets (combined makes the pool size)
            VkDescriptorPoolSize poolSize = {};
            poolSize.type = type;
            poolSize.descriptorCount = count;
            this->descriptorPoolSizes.push_back(poolSize);
        }

        void create(uint32_t maxSets) {

            // Data to create Descriptor Pool
            VkDescriptorPoolCreateInfo poolCreateInfo = {};
            poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolCreateInfo.maxSets = maxSets; // Maximum number of descriptor Sets that cam be create from pool
            poolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()); // Amount of Pool Sizes being passed
            poolCreateInfo.pPoolSizes = descriptorPoolSizes.data();                           // Pool Sizes to create pool with

            // Create Descriptor Pool
            VkResult result = vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &this->descriptorPool);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to create Descriptor pool");
            }
        }

        VkDescriptorPool& getDescriptorPool() { return descriptorPool; }

      private:
        VkDevice device;
        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
    };
} // namespace ce
