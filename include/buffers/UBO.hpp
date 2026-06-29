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
        explicit UBO(VkPhysicalDevice physical, VkDevice logical, const size_t maxUBO, const size_t sizeDataUBO) : logical(logical) {

            // ViewProjection Buffer size
            VkDeviceSize vpBufferSize = sizeDataUBO; // tamanho do struct com os dados

            // One uniform buffer for each image (and by extention, command buffer)
            this->ubo.resize(maxUBO); // total a ser criado

            // Create Unifor buffers
            for (size_t i = 0; i < maxUBO; i++) {
                this->ubo[i] = std::make_shared<BufferObject>(physical, logical);
                this->ubo[i]->create(vpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            }

            // UNIFORM VALUES DESCRIPTOR SET LAYOUT AND DESCRIPTORSETS
            this->descriptorSetLayout = std::make_shared<DescriptorSetLayout>(this->logical);
            this->descriptorSets = std::make_shared<DescriptorSet>(this->logical);
        }

        virtual ~UBO() {

            // for (size_t i = 0; i < sizeDataUBO; i++) {
            //     this->ubo[i].reset();
            // }

            this->descriptorSets.reset(); // TODO: testar??
            this->descriptorSetLayout.reset();
        }

        void addDescriptorSetLayoutBinding(const VkDescriptorSetLayoutBinding& vpLayoutBinding) {
            this->descriptorSetLayout->addBinding(vpLayoutBinding);
        }

        void createDescriptorSetLayout() { this->descriptorSetLayout->create(); }

        void addWriteDescriptorSet(const VkWriteDescriptorSet& vpSetWrite) { this->setWrites.push_back(vpSetWrite); }
        void clearWriteDescriptorSet() { this->setWrites.clear(); }

        void allocateDescriptorSets(const VkDescriptorPool& descriptorPool) {
            std::vector<VkDescriptorSetLayout> setLayouts(this->ubo.size(), this->descriptorSetLayout->get());
            this->descriptorSets->allocate(descriptorPool, setLayouts);
        }

        void updateDescriptorSets() {
            // Update the descripto sets with new buffer/binding info
            vkUpdateDescriptorSets(logical, static_cast<uint32_t>(this->setWrites.size()), this->setWrites.data(), 0, nullptr);
        }

        [[nodiscard]] size_t size() const noexcept { return ubo.size(); }
        [[nodiscard]] std::vector<std::shared_ptr<BufferObject>>& getUBO() { return ubo; }
        [[nodiscard]] VkDescriptorSetLayout& getDescriptorSetLayout() const { return descriptorSetLayout->get(); }
        [[nodiscard]] std::vector<VkDescriptorSet>& getDescriptorSets() const { return descriptorSets->get(); }

      private:
        VkDevice logical;
        std::shared_ptr<DescriptorSet> descriptorSets;
        std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
        std::vector<std::shared_ptr<BufferObject>> ubo;

        std::vector<VkWriteDescriptorSet> setWrites;
    };

} // namespace ce
