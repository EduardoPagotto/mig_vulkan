#pragma once

#include "ShaderModule.hpp"
#include <memory>

namespace ce {

    class PipelineLayout {
      public:
        explicit PipelineLayout(VkDevice device) : device(device) {
            //
        }

        virtual ~PipelineLayout() {
            vkDestroyPipeline(device, this->graphicsPipeline, nullptr);
            vkDestroyPipelineLayout(device, this->pipelineLayout, nullptr);
        }

        void addLayout(VkDescriptorSetLayout& descriptorSetLayout) { this->descriptorSetLayouts.push_back(descriptorSetLayout); }
        void addPushRange(VkPushConstantRange& pushConstantRange) { this->pushConstantRanges.push_back(pushConstantRange); }
        void addViewport(const VkViewport& viewport) { this->viewports.push_back(viewport); }
        void addScissor(const VkRect2D scissor) { this->scissors.push_back(scissor); }
        void addDynamicStateEnables(const VkDynamicState& state) { dynamicStateEnables.push_back(state); };
        void addColourState(const VkPipelineColorBlendAttachmentState& colourState) { this->colourStates.push_back(colourState); }

        void create(std::shared_ptr<ShaderModule> shaderModule, VkRenderPass renderPass) { // NOLINT

            // PipelineLayout
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

            // -- VIEWPORT & SCISSOR
            VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
            viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportStateCreateInfo.viewportCount = static_cast<uint32_t>(this->viewports.size());
            viewportStateCreateInfo.pViewports = this->viewports.data();
            viewportStateCreateInfo.scissorCount = static_cast<uint32_t>(this->scissors.size());
            viewportStateCreateInfo.pScissors = this->scissors.data();

            // Dynamic State creation info
            VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
            dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
            dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();

            // -- RASTERIZER --
            VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};
            rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizationCreateInfo.depthClampEnable =
                VK_FALSE; // Change if fragments beond near/far planes are clipped (default) of clamped to plane
            rasterizationCreateInfo.rasterizerDiscardEnable =
                VK_FALSE; // Whether to diacard data and skip rasterization. Never
                          // create fragments, only suitable for pipeline without frambuffer output
            rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;          // How to handle filling points beteen vertices
            rasterizationCreateInfo.lineWidth = 1.0F;                            // How thick lines shoud be when draw
            rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;            // Whitch face of a tri to cull(nao desenha a backface)
            rasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // kinding to determine side is front
            rasterizationCreateInfo.depthBiasClamp =
                VK_FALSE; // Whether to add depth bias to fragment (good for stopping "swadow acne" in shadow mapping)

            // -- MULTISMAPLING --
            VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
            multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;               // Enable multisample shading or not
            multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Number of sample to use per fragment

            // -- BLENDING --
            // Blending decide how to blend a new colour being written to a fragment, whit the old value
            VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
            colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendingCreateInfo.logicOpEnable = VK_FALSE; // alternative to calulation is use logical operations
            colorBlendingCreateInfo.attachmentCount = static_cast<uint32_t>(this->colourStates.size()); // 1;
            colorBlendingCreateInfo.pAttachments = this->colourStates.data();

            // -- DEPTH STENCIL TESTING
            VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
            depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilCreateInfo.depthTestEnable = VK_TRUE;           // Enable checking depth to determine fragment write
            depthStencilCreateInfo.depthWriteEnable = VK_TRUE;          // Enable writing to depth buffer (to replace all values)
            depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS; // Coparison operation that allows an overwrite (is in front)
            depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;    // Depth Bonds test: Does the depth value exist between two bounds
            depthStencilCreateInfo.stencilTestEnable = VK_FALSE;        // Enable stencil test

            // -- GRAPHICS PIPELINE CREATION
            VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
            pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderModule->getShaderCreateInfos().size()); // numberr of shader stages
            pipelineCreateInfo.pStages = shaderModule->getShaderCreateInfos().data();                           // List of shader stages
            pipelineCreateInfo.pVertexInputState = shaderModule->getpVertexInputCreateInfo(); // All the fixed function pipeline states
            pipelineCreateInfo.pInputAssemblyState = shaderModule->getpInputAssembly();
            pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
            pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo; // nullptr;
            pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
            pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
            pipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
            pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
            pipelineCreateInfo.layout = pipelineLayout; // Pipeline Layout
                                                        // pipeline shoud use
            pipelineCreateInfo.renderPass = renderPass; // Render pass description the
            ;                                           // pipelineis compatible whit
            pipelineCreateInfo.subpass = 0;             // Subpass of render pass to use with pipeline

            // Pipeline Derivatives : Can create multiple pipelines that derive from one another for optimisation
            pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Existing pipeline to derive from...
            pipelineCreateInfo.basePipelineIndex = -1;              // or index of pipeline being created to derive from
            ;                                                       // (in case creating multiple at once)

            // Create Graphics Pipeline
            result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &this->graphicsPipeline);

            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to create a graphic pipeline");
            }
        }

        VkPipelineLayout& getPipelineLayout() { return this->pipelineLayout; }
        VkPipeline& getGraphicsPipeline() { return this->graphicsPipeline; }

      private:
        VkDevice device;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        std::vector<VkPushConstantRange> pushConstantRanges;
        std::vector<VkViewport> viewports;
        std::vector<VkRect2D> scissors;
        std::vector<VkDynamicState> dynamicStateEnables;
        std::vector<VkPipelineColorBlendAttachmentState> colourStates;
    };
} // namespace ce
