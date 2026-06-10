#pragma once

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ce {

    class ShaderModule {
      public:
        explicit ShaderModule(VkDevice device, VkShaderStageFlagBits stage, const std::vector<char>& code) : device(device) {

            VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
            shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shaderModuleCreateInfo.codeSize = code.size();                                 // size of code
            shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); // pointer to code(of uint32_t pointer type)

            VkResult result = vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &this->shaderModule);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to create a shader module");
            }

            shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderCreateInfo.stage = stage;               // Shader stage name
            shaderCreateInfo.module = this->shaderModule; // Shader module to be used by stage
            shaderCreateInfo.pName = "main";              // Entry point in to shader
        }

        virtual ~ShaderModule() { vkDestroyShaderModule(device, shaderModule, nullptr); }

        VkShaderModule& getModule() { return this->shaderModule; }
        VkPipelineShaderStageCreateInfo& getShaderCreateInfo() { return this->shaderCreateInfo; }

      private:
        VkDevice device;
        VkShaderModule shaderModule;
        VkPipelineShaderStageCreateInfo shaderCreateInfo = {};
    };
} // namespace ce
