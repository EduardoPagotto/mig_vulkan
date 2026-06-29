#pragma once

#include "buffers/BufferObject.hpp"
#include "descriptors/DescriptorSet.hpp"
#include "descriptors/DescriptorSetLayout.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ce {

    class UBO {
      public:
        explicit UBO(const size_t maxUBO, const size_t sizeDataUBO) : sizeDataUBO(sizeDataUBO) {

            // ViewProjection Buffer size
            VkDeviceSize vpBufferSize = sizeDataUBO; // tamanho do struct com os dados

            // One uniform buffer for each image (and by extention, command buffer)
            this->vpUniformBuffer.resize(maxUBO); // total a ser criado

            // Create Unifor buffers
            for (size_t i = 0; i < maxUBO; i++) {
                this->vpUniformBuffer[i] = std::make_shared<BufferObject>(physical, logical);
                this->vpUniformBuffer[i]->create(vpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            }

            // UNIFORM VALUES DESCRIPTOR SET LAYOUT AND DESCRIPTORSETS
            this->descriptorSetLayout = std::make_shared<DescriptorSetLayout>(this->logical);
            this->descriptorSets = std::make_shared<DescriptorSet>(this->logical);
        }

        virtual ~UBO() {

            this->descriptorSetLayout.reset();
            this->descriptorSets.reset(); // TODO: testar??
            for (size_t i = 0; i < sizeDataUBO; i++) {
                this->vpUniformBuffer[i].reset();
            }
        }

        void addDescriptorSetLayout(const VkDescriptorSetLayoutBinding& vpLayoutBinding) {
            this->descriptorSetLayout->addBinding(vpLayoutBinding);
        }

        void createDescriptorSetLayout() { this->descriptorSetLayout->create(); }

        void addWriteDescriptorSet(const VkWriteDescriptorSet& vpSetWrite) { this->setWrites.push_back(vpSetWrite); }

        void createDescriptorSets(const VkDescriptorPool& descriptorPool) {

            std::vector<VkDescriptorSetLayout> setLayouts(this->vpUniformBuffer.size(), this->descriptorSetLayout->get());
            this->descriptorSets->allocate(descriptorPool, setLayouts);

            // Update all of descriptor set buffer bindings
            for (size_t i = 0; i < sizeDataUBO; i++) {
                // Update the descripto sets with new buffer/binding info
                vkUpdateDescriptorSets(logical, static_cast<uint32_t>(this->setWrites.size()), this->setWrites.data(), 0, nullptr);
            }
        }

        std::vector<std::shared_ptr<BufferObject>>& getUBO() { return vpUniformBuffer; }

      private:
        size_t sizeDataUBO;

        VkPhysicalDevice physical;
        VkDevice logical;

        std::shared_ptr<DescriptorSet> descriptorSets;
        std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
        std::vector<std::shared_ptr<BufferObject>> vpUniformBuffer;

        std::vector<VkWriteDescriptorSet> setWrites;
    };

} // namespace ce
