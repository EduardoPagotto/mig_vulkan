#pragma once

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ce {

    class PipelineLayout {
      public:
        explicit PipelineLayout(VkDevice device) : device(device) {
            //
        }

        virtual ~PipelineLayout() { vkDestroyPipelineLayout(device, this->pipelineLayout, nullptr); }

        void addLayout(VkDescriptorSetLayout& descriptorSetLayout) { this->descriptorSetLayouts.push_back(descriptorSetLayout); }
        void addPushRange(VkPushConstantRange& pushConstantRange) { this->pushConstantRanges.push_back(pushConstantRange); }

        void create() {

            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
            pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
            pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
            pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(this->pushConstantRanges.size());
            pipelineLayoutCreateInfo.pPushConstantRanges = this->pushConstantRanges.data();

            // Create PipelineLayout
            VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &this->pipelineLayout);

            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to create Pipeline Layout!");
            }
        }

        VkPipelineLayout& getPipelineLayout() { return this->pipelineLayout; }

      private:
        VkDevice device;
        VkPipelineLayout pipelineLayout;
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        std::vector<VkPushConstantRange> pushConstantRanges;
    };
} // namespace ce
