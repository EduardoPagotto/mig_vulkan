#pragma once

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ce {

    class DescriptorSetLayout {
      public:
        explicit DescriptorSetLayout(VkDevice device) : device(device) {
            //
        }

        virtual ~DescriptorSetLayout() { vkDestroyDescriptorSetLayout(device, this->descriptorSetLayout, nullptr); }

        void addBinding(const VkDescriptorSetLayoutBinding& vpLayoutBinding) { this->layoutBinding.push_back(vpLayoutBinding); }

        void create() {

            // Create Desciptor Set Layout with given bindingd
            VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
            layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBinding.size()); // Number of binding infos
            layoutCreateInfo.pBindings = layoutBinding.data();                           // Array of binding infos

            // Create Descriptor Set Layout
            VkResult result = vkCreateDescriptorSetLayout(this->device, &layoutCreateInfo, nullptr, &this->descriptorSetLayout);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to create descriptor set Layout!");
            }
        }

        VkDescriptorSetLayout& getDescriptorSetLayout() { return this->descriptorSetLayout; }

        // std::vector<VkDescriptorSetLayoutBinding>& getLayoutBinding() { return layoutBinding; }

      private:
        VkDevice device;
        VkDescriptorSetLayout descriptorSetLayout;

        std::vector<VkDescriptorSetLayoutBinding> layoutBinding;
    };
} // namespace ce
