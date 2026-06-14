#pragma once

#include <stdexcept>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ce {

    class DescriptorPool {
      public:
        explicit DescriptorPool(VkDevice device) : device(device) {}

        virtual ~DescriptorPool() { cleanup(); }

        void addPoolSize(const VkDescriptorType& type, const uint32_t& count) {
            // Type of Descriptors + how many DESCRIPTORS, not Descriptor Sets (combined makes the pool size)
            this->poolSize.push_back(VkDescriptorPoolSize{.type = type, .descriptorCount = count});
        }

        void create(const uint32_t& maxSets) {

            // Data to create Descriptor Pool
            const VkDescriptorPoolCreateInfo poolCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .maxSets = maxSets,                                      // Maximum number of descriptor Sets that can be create from pool
                .poolSizeCount = static_cast<uint32_t>(poolSize.size()), // Amount of Pool Sizes being passed
                .pPoolSizes = poolSize.data()                            // Pool Sizes to create pool with
            };

            if (vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &this->handle) != VK_SUCCESS) { // Create Descriptor Pool
                throw std::runtime_error("Failed to create Descriptor pool");
            }

            poolSize.clear();
            poolSize.shrink_to_fit();
        }

        void cleanup() noexcept {
            if (handle != VK_NULL_HANDLE && device != VK_NULL_HANDLE) {
                vkDestroyDescriptorPool(device, handle, nullptr);
                handle = VK_NULL_HANDLE;
            }
        }

        // Operador de conversão implícita para facilitar o uso nas funções do Vulkan
        explicit operator VkDescriptorPool() const noexcept { return handle; }

        [[nodiscard]] VkDescriptorPool& get() { return handle; }

        // Desabilita cópias para evitar dupla destruição do mesmo handle
        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        // Habilita movimento (Move Semantics) para transferir propriedade
        DescriptorPool(DescriptorPool&& other) noexcept
            : device{std::exchange(other.device, VK_NULL_HANDLE)}, handle{std::exchange(other.handle, VK_NULL_HANDLE)} {}

        DescriptorPool& operator=(DescriptorPool&& other) noexcept {
            if (this != &other) {
                cleanup(); // Destrói o recurso atual antes de assumir o novo
                device = std::exchange(other.device, VK_NULL_HANDLE);
                handle = std::exchange(other.handle, VK_NULL_HANDLE);
            }
            return *this;
        }

      private:
        VkDevice device{VK_NULL_HANDLE};
        VkDescriptorPool handle{VK_NULL_HANDLE};
        std::vector<VkDescriptorPoolSize> poolSize;
    };
} // namespace ce
