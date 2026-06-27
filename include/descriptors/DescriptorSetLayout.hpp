#pragma once

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ce {

    class DescriptorSetLayout {
      public:
        explicit DescriptorSetLayout(VkDevice device) : device(device) {}

        virtual ~DescriptorSetLayout() {
            if (handle != VK_NULL_HANDLE && device != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(device, this->handle, nullptr);
            }
        }

        void addBinding(const VkDescriptorSetLayoutBinding& vpLayoutBinding) { this->layoutBinding.push_back(vpLayoutBinding); }

        void create() {

            // Create Desciptor Set Layout with given bindingd
            const VkDescriptorSetLayoutCreateInfo layoutCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<uint32_t>(layoutBinding.size()), // Number of binding infos
                .pBindings = layoutBinding.data()                            // Array of binding infos
            };

            // Create Descriptor Set Layout
            if (vkCreateDescriptorSetLayout(this->device, &layoutCreateInfo, nullptr, &this->handle) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create descriptor set Layout!");
            }

            layoutBinding.clear();
            layoutBinding.shrink_to_fit();
        }

        [[nodiscard]] VkDescriptorSetLayout& get() { return this->handle; }

      private:
        VkDevice device{VK_NULL_HANDLE};
        VkDescriptorSetLayout handle{VK_NULL_HANDLE};
        std::vector<VkDescriptorSetLayoutBinding> layoutBinding;
    };
} // namespace ce
