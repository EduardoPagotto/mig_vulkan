#pragma once

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ce {

    class DescriptorSet {
      public:
        explicit DescriptorSet(VkDevice device) : device(device) {
            //
        }

        virtual ~DescriptorSet() {
            //
        }

        std::pair<size_t, size_t> allocate(VkDescriptorPool& descriptorPool, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) {

            // Reserve new spaces in descriptorSet
            size_t startPosition = this->descriptorSets.size();
            size_t increment = descriptorSetLayouts.size(); // Total of descriptors to allocate increment
            // size_t finalTot = startPosition + increment;        // incrize total alocation of  descriptorSets
            //  this->descriptorSets.resize(finalTot);          // new size of descriptorSets

            // Reserve space to temprary desciptor
            std::vector<VkDescriptorSet> localSets(increment);

            // Descriptor Set Allocation info
            VkDescriptorSetAllocateInfo setAllocInfo = {};
            setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            setAllocInfo.descriptorPool = descriptorPool;                       // Pool to allocate Descriptor Set
            setAllocInfo.descriptorSetCount = static_cast<uint32_t>(increment); // Number of sets to allocate
            setAllocInfo.pSetLayouts = descriptorSetLayouts.data();             // Layouts to use to allocate sets (1:1 relationship)

            // Allocate descriptor sets (multiple)
            VkResult result = vkAllocateDescriptorSets(device, &setAllocInfo, localSets.data());
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate Texture descriptor set");
            }

            // copy localSets sets to descriptorSets
            this->descriptorSets.insert(this->descriptorSets.end(), localSets.begin(), localSets.end());

            return {startPosition, increment}; // start position, total new allocate
        }

        std::vector<VkDescriptorSet>& getDescriptorSets() { return this->descriptorSets; }

      private:
        VkDevice device;
        std::vector<VkDescriptorSet> descriptorSets;
    };
} // namespace ce
