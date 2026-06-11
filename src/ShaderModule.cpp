#include "ShaderModule.hpp"
#include <stdexcept>

namespace ce {
    ShaderModule::~ShaderModule() {
        for (size_t i = 0; i < this->shaderModules.size(); i++) {
            vkDestroyShaderModule(device, shaderModules[i], nullptr);
        }
    }

    void ShaderModule::addCode(VkShaderStageFlagBits stage, const std::vector<char>& code) {

        VkShaderModule shaderModule = {};
        size_t pos = shaderModules.size();

        shaderModules.push_back(shaderModule);

        VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = code.size();                                 // size of code
        shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); // pointer to code(of uint32_t pointer type)

        VkResult result = vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModules[pos]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create a shader module");
        }

        VkPipelineShaderStageCreateInfo shaderCreateInfo;
        shaderCreateInfos.push_back(shaderCreateInfo);

        shaderCreateInfos[pos].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderCreateInfos[pos].stage = stage;               // Shader stage name
        shaderCreateInfos[pos].module = shaderModules[pos]; // Shader module to be used by stage
        shaderCreateInfos[pos].pName = "main";              // Entry point in to shader
    }

    void ShaderModule::addAtribute(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset) {
        //
        VkVertexInputAttributeDescription attribute = {};
        size_t pos = attributeDescriptions.size();
        attributeDescriptions.push_back(attribute);

        attributeDescriptions[pos].binding = binding;   // Which binding the data is at (should be sdame as above)
        attributeDescriptions[pos].location = location; // Location in shader where data will be read from
        attributeDescriptions[pos].format = format;     // Forma the data will take (also helps define size of data)
        attributeDescriptions[pos].offset = offset;     // Where this attribute is defined in the data for a single vertex
    }

    void ShaderModule::addBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate) {
        //
        VkVertexInputBindingDescription bindingDescription = {};
        size_t pos = bindingDescriptions.size();
        bindingDescriptions.push_back(bindingDescription);

        bindingDescriptions[pos].binding = binding;     // Cam bind multiple streams of data, thos defines which one
        bindingDescriptions[pos].stride = stride;       // Size of a single vertex object
        bindingDescriptions[pos].inputRate = inputRate; // How to move between data after each vertex
        ;                                               // VK_VERTEX_INPUT_RATE_INDEX : Move on to the next vertex
        ;                                               // VK_VERTEX_INPUT_RATR_INSTANCE: Move to a vertex for the next instance
    }

    void ShaderModule::setVertexInput(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable) {
        //
        // -- VERTEX INPUT --
        vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescriptions.data(); // List of vertex bind Descritions
        ;                                                                              // (data spacing stride information)
        vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(this->attributeDescriptions.size());
        vertexInputCreateInfo.pVertexAttributeDescriptions = this->attributeDescriptions.data(); // Listof Vertex Attribute Description
        ;                                                                                        //  (data format and where
        ;                                                                                        // to bind to/from)

        //
        // -- INPUT ASSEMBLY --
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = topology;                             // Primitive type to assemple vertice as
        inputAssembly.primitiveRestartEnable = primitiveRestartEnable; // Allow overiding of "strip" topology to start new primitive
    }
} // namespace ce
