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
        void addViewport(const VkViewport& viewport) { this->viewports.push_back(viewport); }
        void addScissor(const VkRect2D scissor) { this->scissors.push_back(scissor); }
        void addColourState(const VkPipelineColorBlendAttachmentState& colourState) { this->colourStates.push_back(colourState); }

        void create() {

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
            viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportStateCreateInfo.viewportCount = static_cast<uint32_t>(this->viewports.size());
            viewportStateCreateInfo.pViewports = this->viewports.data();
            viewportStateCreateInfo.scissorCount = static_cast<uint32_t>(this->scissors.size());
            viewportStateCreateInfo.pScissors = this->scissors.data();

            // -- RASTERIZER --
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
            multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;               // Enable multisample shading or not
            multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Number of sample to use per fragment

            // -- BLENDING --
            // Blending decide how to blend a new colour being written to a fragment, whit the old value
            colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendingCreateInfo.logicOpEnable = VK_FALSE; // alternative to calulation is use logical operations
            colorBlendingCreateInfo.attachmentCount = static_cast<uint32_t>(this->colourStates.size()); // 1;
            colorBlendingCreateInfo.pAttachments = this->colourStates.data();

            // -- DEPTH STENCIL TESTING
            depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilCreateInfo.depthTestEnable = VK_TRUE;           // Enable checking depth to determine fragment write
            depthStencilCreateInfo.depthWriteEnable = VK_TRUE;          // Enable writing to depth buffer (to replace all values)
            depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS; // Coparison operation that allows an overwrite (is in front)
            depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;    // Depth Bonds test: Does the depth value exist between two bounds
            depthStencilCreateInfo.stencilTestEnable = VK_FALSE;        // Enable stencil test

            // // -- GRAPHICS PIPELINE CREATION
            // VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
            // pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            // pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderModule->getShaderCreateInfos().size()); // numberr of shader
            // stages pipelineCreateInfo.pStages = shaderModule->getShaderCreateInfos().data();                           // List of shader
            // stages pipelineCreateInfo.pVertexInputState = shaderModule->getpVertexInputCreateInfo(); // All the fixed function pipeline
            // states pipelineCreateInfo.pInputAssemblyState = shaderModule->getpInputAssembly(); pipelineCreateInfo.pViewportState =
            // this->pipelineLayout->getpViewportStateCreateInfo(); pipelineCreateInfo.pDynamicState = nullptr;
            // pipelineCreateInfo.pRasterizationState = this->pipelineLayout->getpRasterizationCreateInfo();
            // pipelineCreateInfo.pMultisampleState = this->pipelineLayout->getpMultisamplingCreateInfo();
            // pipelineCreateInfo.pColorBlendState = this->pipelineLayout->getpColorBlendingCreateInfo();  //&colorBlendingCreateInfo;
            // pipelineCreateInfo.pDepthStencilState = this->pipelineLayout->getPDepthStencilCreateInfo(); //&depthStencilCreateInfo;
            // pipelineCreateInfo.layout = this->pipelineLayout->getPipelineLayout(); // pipelineLayout;                     // Pipeline
            // Layout
            //                                                                        // pipeline shoud use
            // pipelineCreateInfo.renderPass = this->rederer->getRenderPass();        // Render pass description the pipelineis compatible
            // whit pipelineCreateInfo.subpass = 0;                                        // Subpass of render pass to use with pipeline

            // // Pipeline Derivatives : Can create multiple pipelines that derive from one another for optimisation
            // pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Existing pipeline to derive from...
            // pipelineCreateInfo.basePipelineIndex =
            //     -1; // or index of pipeline being created to derive from (in case creating multiple at once)

            // // Create Graphics Pipeline
            // result =
            //     vkCreateGraphicsPipelines(vwrapp->getLogical(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr,
            //     &this->graphicsPipeline);

            // if (result != VK_SUCCESS) {
            //     throw std::runtime_error("Failed to create a graphic pipeline");
            // }
        }

        VkPipelineLayout& getPipelineLayout() { return this->pipelineLayout; }

        std::vector<VkViewport>& getViewports() { return this->viewports; }
        std::vector<VkRect2D>& getScissors() { return this->scissors; }
        VkPipelineViewportStateCreateInfo* getpViewportStateCreateInfo() { return &viewportStateCreateInfo; }

        VkPipelineRasterizationStateCreateInfo* getpRasterizationCreateInfo() { return &rasterizationCreateInfo; }
        VkPipelineMultisampleStateCreateInfo* getpMultisamplingCreateInfo() { return &multisamplingCreateInfo; }

        VkPipelineColorBlendStateCreateInfo* getpColorBlendingCreateInfo() { return &colorBlendingCreateInfo; }

        VkPipelineDepthStencilStateCreateInfo* getPDepthStencilCreateInfo() { return &depthStencilCreateInfo; }

      private:
        VkDevice device;
        VkPipelineLayout pipelineLayout;
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        std::vector<VkPushConstantRange> pushConstantRanges;

        std::vector<VkViewport> viewports;
        std::vector<VkRect2D> scissors;
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};

        VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};

        VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};

        std::vector<VkPipelineColorBlendAttachmentState> colourStates;
        VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};

        VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
    };
} // namespace ce
