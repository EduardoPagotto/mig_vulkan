#include "VulkanRenderer.hpp"
#include "Mesh.hpp"
#include "ShaderModule.hpp"
#include "Ultilities.hpp"
#include "VWrappUtils.hpp"
#include "buffers/CommandBuffer.hpp"
#include "buffers/utils.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/trigonometric.hpp>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

VulkanRenderer::VulkanRenderer(std::shared_ptr<ce::VWrapp> vwrapp) : vwrapp(vwrapp) { // NOLINT

    this->swapchain =
        std::make_shared<ce::SwapChain>(vwrapp->getPhysical(), vwrapp->getLogical(), vwrapp->getSurface(), vwrapp->getWindow());

    this->rederer = std::make_shared<ce::Renderer>(vwrapp->getPhysical(), vwrapp->getLogical(), this->swapchain->getImageFormat());

    // createUniformBuffers
    this->ubo = std::make_shared<ce::UBO>(this->vwrapp->getPhysical(), this->vwrapp->getLogical(), this->swapchain->getImages().size(),
                                          sizeof(UboViewProjection));
    this->createDescriptorSetLayout();
    this->createPushConstantRange();
    this->createGraphicsPipeline();
    this->createDepthBufferImage();

    this->swapchain->createFramebuffers(this->depthBufferObject->getImageView(), this->rederer->getRenderPass());
    this->graphicsCommandPool = std::make_shared<ce::CommandPool>(vwrapp->getPhysical(), vwrapp->getLogical(), vwrapp->getSurface());

    // In create Command buffer, count to have one for each frambuffer
    this->commandBuffers = std::make_shared<ce::CommandBuffer>(vwrapp->getLogical(), this->graphicsCommandPool->getPool(),
                                                               this->swapchain->getSwapChainFrameBuffers().size());
    this->createTextureSampler();
    // this->allocateDynamicBufferTransferSpace();
    this->createDescriptorPool();
    this->createDescriptorSets();
    this->createSynchronisation();

    // const float radixAngle = 45.0F;
    const float near = 0.1F;
    const float far = 1000.0F;

    const float radixAngle = glm::radians(45.F); // 1:15:21
    const glm::vec3 camPos = glm::vec3(-100.0F, 150.0F, 200.0F);
    const glm::vec3 camCenter = glm::vec3(0.0F, 0.0F, -2.0F);
    const glm::vec3 camUp = glm::vec3(0.0F, 1.0F, 0.0F);
    const float aspect = (float)this->swapchain->getExtent().width / (float)this->swapchain->getExtent().height;

    this->uboViewProjection.projection = glm::perspective(radixAngle, aspect, near, far);
    this->uboViewProjection.view = glm::lookAt(camPos, camCenter, camUp);

    this->uboViewProjection.projection[1][1] *= -1; // vulkan inverted of OpenGL

    // Create our default "no texture" texture
    createTexture("plain.png");
}

VulkanRenderer::~VulkanRenderer() {

    // Wait until no action being run on device before destroying
    vkDeviceWaitIdle(vwrapp->getLogical());

    // free(this->modelTransferSpace);
    for (auto& model : this->modelList) {
        model.destroyMeshModel();
    }

    this->samplerDescriptorPool.reset();
    this->samplerSetLayout.reset();

    vkDestroySampler(vwrapp->getLogical(), this->textureSampler, nullptr);

    for (size_t i = 0; i < this->textureImageObjects.size(); i++) {
        this->textureImageObjects[i].reset();
    }

    this->depthBufferObject.reset();
    this->descriptorPool.reset();
    this->ubo.reset();

    for (size_t i = 0; i < MAX_FRAME_DRAWS; i++) {

        vkDestroySemaphore(vwrapp->getLogical(), this->renderFinished[i], nullptr);
        vkDestroySemaphore(vwrapp->getLogical(), this->imageAvailable[i], nullptr);
        vkDestroyFence(vwrapp->getLogical(), this->drawFences[i], nullptr);
    }

    this->commandBuffers.reset();
    this->graphicsCommandPool.reset();
    this->pipeline.reset();
}

void VulkanRenderer::updateModel(int modelId, glm::mat4 newModel) {

    if (modelId >= this->modelList.size()) {
        return;
    }

    this->modelList[modelId].setModel(newModel);
}

void VulkanRenderer::draw() {
    // -- GET NEXT IMAGE --
    // Wait for given fence to signal (open) from last draw before continuing
    vkWaitForFences(vwrapp->getLogical(), 1, &this->drawFences[this->currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    // Manually reset (close) fence
    vkResetFences(vwrapp->getLogical(), 1, &this->drawFences[this->currentFrame]);

    // Get index of next image to be draw to, and signal semaphore when ready to be draw to
    uint32_t imageIndex;
    vkAcquireNextImageKHR(vwrapp->getLogical(), this->swapchain->getKHR(), std::numeric_limits<uint64_t>::max(),
                          this->imageAvailable[this->currentFrame], VK_NULL_HANDLE, &imageIndex);

    this->recordCommands(imageIndex);
    this->updateUniformBuffers(imageIndex);

    // -- SUBMIT COMMAND BUFFER TO RENDER
    // Queue submission information
    std::array<VkSemaphore, 1> waitSemaphores{this->imageAvailable[this->currentFrame]};
    std::array<VkSemaphore, 1> signalSemaphores{this->renderFinished[this->currentFrame]};

    std::array<VkPipelineStageFlags, 1> waitStages{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),     // Number of semaphores to wait on
        .pWaitSemaphores = waitSemaphores.data(),                               //
        .pWaitDstStageMask = waitStages.data(),                                 // Stagegs to check semaphores at
        .commandBufferCount = 1,                                                // Number of command buffers to submit FIXME: é isto mesmo?
        .pCommandBuffers = &this->commandBuffers->getBuffers()[imageIndex],     // Command buffer to submit
        .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()), // Number of semaphore to signal
        .pSignalSemaphores = signalSemaphores.data(),                           // Semaphore to signal when command buffer finishes
    };

    // Submit command buffer to queue
    if (vkQueueSubmit(vwrapp->getGraphicsQueue(), 1, &submitInfo, this->drawFences[this->currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit Command Buffer to Queue!");
    }

    // -- PRESENT RENDERED IMAGE TO SCREEN --
    std::array<VkSwapchainKHR, 1> swapChains{this->swapchain->getKHR()};
    const VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()), // Number of semaphores to wait on
        .pWaitSemaphores = signalSemaphores.data(),                           // Semaphores to wait on
        .swapchainCount = static_cast<uint32_t>(swapChains.size()),           // Number of swapchains to present to
        .pSwapchains = swapChains.data(),                                     // Swapchais to present images to
        .pImageIndices = &imageIndex,                                         // Index of Images in swapchains to present
    };

    // Present Image
    if (vkQueuePresentKHR(vwrapp->getPresentationQueue(), &presentInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to present Image!");
    }

    // Get next frame
    this->currentFrame = (this->currentFrame + 1) % MAX_FRAME_DRAWS;

    // AHHHH!!!!!! ugly!!!!! this is complete wrong, find what missmatch sYncs!!!
    if (this->currentFrame == (MAX_FRAME_DRAWS - 1)) {
        vkDeviceWaitIdle(vwrapp->getLogical());
    }
}

void VulkanRenderer::createDescriptorSetLayout() {

    // UNIFORM VALUES DESCRIPTOR SET LAYOUT
    // UboViewProjection Binding info
    this->ubo->addDescriptorSetLayoutBinding({
        .binding = 0,                                        // Binding point in shader (designed by binding number in shader)
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // Type of descriptor (uniform, dynamic, image sampler, etc)
        .descriptorCount = 1,                                // Number of descriptors for binding
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,            // Shade stage to bind to
        .pImmutableSamplers = nullptr, // for Texture: can make sampler unchangeable (immutable) by specifying in layout
    });

    // // Model Binding Info
    // this->descriptorSetLayout->addBinding({.binding = 1,
    //                                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
    //                                        .descriptorCount = 1,
    //                                        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    //                                        .pImmutableSamplers = nullptr});

    this->ubo->createDescriptorSetLayout();

    // CREATE TEXTURE SAMPLER DESCRIPTOR SET LAYOUT
    this->samplerSetLayout = std::make_shared<ce::DescriptorSetLayout>(this->vwrapp->getLogical());

    // Texture binding info
    this->samplerSetLayout->addBinding({.binding = 0,
                                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                        .descriptorCount = 1,
                                        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                                        .pImmutableSamplers = nullptr});

    this->samplerSetLayout->create();
}

void VulkanRenderer::createPushConstantRange() {
    // Define push constant value (no 'create' needed!)
    this->pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Shader stage push constant will go to
    this->pushConstantRange.offset = 0;                              // offset into given data to pass to push constant
    this->pushConstantRange.size = sizeof(Model);                    // Size of data being passed
}

void VulkanRenderer::createGraphicsPipeline() {

    // Read in SPIR-V code shaders, Vertex Stage creation information and Fragment Stage creation information
    std::shared_ptr<ce::ShaderModule> shaderModule = std::make_shared<ce::ShaderModule>(vwrapp->getLogical());
    shaderModule->addCode(VK_SHADER_STAGE_VERTEX_BIT, readFile("./bin/vert.spv"));
    shaderModule->addCode(VK_SHADER_STAGE_FRAGMENT_BIT, readFile("./bin/frag.spv"));

    // How the data for a sigle vertex (including info such as position, colour, texture coords, normals, etc..) is as a whole
    shaderModule->addBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

    // Attributes of shader vertex
    shaderModule->addAtribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)); // Position Attribute
    shaderModule->addAtribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, col)); // Color Attribute
    shaderModule->addAtribute(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, tex));    // Texture Atribute

    // -- VERTEX INPUT  ASSEMBLY INPUT --
    shaderModule->setVertexInput(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);

    // -- VIEWPORT & SCISSOR
    const VkViewport viewport{.x = 0.0F,                                            // x start coordinate
                              .y = 0.0F,                                            // y start coordinate
                              .width = (float)this->swapchain->getExtent().width,   // width of viewport
                              .height = (float)this->swapchain->getExtent().height, // height of viewport
                              .minDepth = 0.0F,                                     // min framebuffer depth
                              .maxDepth = 1.0F};                                    // max framebuffer depth

    const VkRect2D scissor{.offset = VkOffset2D{.x = 0, .y = 0},    // Offset to use region from
                           .extent = this->swapchain->getExtent()}; // Extent to describe region to use, starting at offset

    // TODO: mudar o nome da classe
    this->pipeline = std::make_shared<ce::Pipeline>(this->vwrapp->getLogical());
    this->pipeline->addViewport(viewport);
    this->pipeline->addScissor(scissor);

    // // -- DYNAMIC STATES --
    // this->pipeline->addDynamicStateEnables(VK_DYNAMIC_STATE_VIEWPORT); // Dynamic Viewport: Can resize in command buffer with
    // ;                                                                        // vkCmdSetViewport(commandbuffer, 0, 1, &viewport);
    // this->pipeline->addDynamicStateEnables(VK_DYNAMIC_STATE_SCISSOR);  // Dynamic Scissor: Can resize in command buffer with
    // ;                                                                        // vkCmdSetViewport(commandbuffer, 0, 1, &scissor);

    // Blend Attachment State (how blending is handled)
    // Blending uses equation: (srcColorBlendfactor * new colour) colorBlendOp (dstColorBlendfactor * old colour)
    // Sumarised 1: (VK_BLEND_FACTOR_SRC_ALPHA * new colour) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old colour)
    //            (new colour alpha * new colour) + ((i - new color alpha) * old colour)
    // Sumarized 2: (1 * new alpha) + (0 * old Alpha) = new alpha

    const VkPipelineColorBlendAttachmentState colourState{
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT, // Color to apply blending to
    };

    this->pipeline->addColourState(colourState);

    // -- PIPELINE LAYOUT --
    this->pipeline->addLayout(this->ubo->getDescriptorSetLayout());
    this->pipeline->addLayout(this->samplerSetLayout->get());
    this->pipeline->addPushRange(this->pushConstantRange);

    // -- GRAPHICS PIPELINE CREATION
    this->pipeline->create(shaderModule, this->rederer->getRenderPass());
}

void VulkanRenderer::createDepthBufferImage() {

    // Get suported format for depth buffer
    VkFormat depthFormat = ce::ChooseSupportedFormat(
        this->vwrapp->getPhysical(), {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT}, // Formats
        VK_IMAGE_TILING_OPTIMAL,                                                                                        // Tilling
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);                                                                // Depth

    // Create Depth Buffer Image
    this->depthBufferObject = std::make_shared<ce::ImageObject>(this->vwrapp->getPhysical(), this->vwrapp->getLogical());
    this->depthBufferObject->createImage(this->swapchain->getExtent().width, this->swapchain->getExtent().height, depthFormat,
                                         VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Create Depth Buffer Image View
    this->depthBufferObject->createImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanRenderer::createSynchronisation() {

    this->imageAvailable.resize(MAX_FRAME_DRAWS);
    this->renderFinished.resize(MAX_FRAME_DRAWS);
    this->drawFences.resize(MAX_FRAME_DRAWS);

    // Semaphore creation information
    const VkSemaphoreCreateInfo semaphoreCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    // Fence creation information
    const VkFenceCreateInfo fenceCreateInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};

    for (size_t i = 0; i < MAX_FRAME_DRAWS; i++) {

        if (vkCreateSemaphore(vwrapp->getLogical(), &semaphoreCreateInfo, nullptr, &this->imageAvailable[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vwrapp->getLogical(), &semaphoreCreateInfo, nullptr, &this->renderFinished[i]) != VK_SUCCESS ||
            vkCreateFence(vwrapp->getLogical(), &fenceCreateInfo, nullptr, &this->drawFences[i]) != VK_SUCCESS) {

            throw std::runtime_error("Failed to create a Semaphore and/or Fence!");
        }
    }
}

void VulkanRenderer::createTextureSampler() {
    // Sampler create info
    const VkSamplerCreateInfo samplerCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,                   // How torender when image is magnified on screen
        .minFilter = VK_FILTER_LINEAR,                   // How to render when image is minifield on screen
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,     // Mipmap interpolation mode
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,  // How to handle texture wrap in U(x) direction
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,  // How to handle texture wrap in V(y) direction
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,  // How to handle texture wrap in W(z) direction
        .mipLodBias = 0.0F,                              // Level of detail of bias for mip level
        .anisotropyEnable = VK_TRUE,                     // Enable anisotropy
        .maxAnisotropy = 16,                             // Anisotropy sample level
        .minLod = 0.0F,                                  // Minimum Level Detail ro pick mip level
        .maxLod = 0.0F,                                  // Maximum Level Detail ro pick mip level
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK, // Border beond texture (only works for border clamp)
        .unnormalizedCoordinates = VK_FALSE,             // Wheter coords should be normalized (between 0 and 1)
    };

    if (vkCreateSampler(vwrapp->getLogical(), &samplerCreateInfo, nullptr, &this->textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a Sampler");
    }
}

void VulkanRenderer::createDescriptorPool() {

    // CREATE DESCRIPTOR POOL
    // CREATE UNIFORM DESCRIPTOR POOL
    this->descriptorPool = std::make_shared<ce::DescriptorPool>(this->vwrapp->getLogical());
    // Type of Descriptors + how many DESCRIPTORS, not Descriptor Sets (combined makes the pool size)
    // ViewProjection Pool
    this->descriptorPool->addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(this->ubo->size()));

    // // Model Pool (Dynamic)
    // this->descriptorPool->addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, //
    //                                    static_cast<uint32_t>(this->modelDUniformBuffer.size());//

    // Create Descriptor Pool
    this->descriptorPool->create(static_cast<uint32_t>(this->swapchain->getImages().size())); // Maximum number of descriptor Sets
    ;

    // -- CREATE UNIFORM DESCRIPTOR POOL
    // Texture sampler pool
    this->samplerDescriptorPool = std::make_shared<ce::DescriptorPool>(this->vwrapp->getLogical());
    this->samplerDescriptorPool->addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_OBJECTS);
    this->samplerDescriptorPool->create(MAX_OBJECTS);
}

void VulkanRenderer::createDescriptorSets() {
    // create smart pointer of Descripor set collection and Resize Descriptor Set list so one for every buffer
    this->samplerDescriptorSets = std::make_shared<ce::DescriptorSet>(this->vwrapp->getLogical());

    this->ubo->allocateDescriptorSets(this->descriptorPool->get());

    // Update all of descriptor set buffer bindings
    for (size_t i = 0; i < this->swapchain->getImages().size(); i++) {
        // VIEW PROJECTION DESCRIPTOR
        // Buffer info and data offset info
        const VkDescriptorBufferInfo vpBufferInfo{
            //.buffer = this->vpUniformBuffer[i]->getBuffer(), // Buffer get data from
            .buffer = this->ubo->getUBO()[i]->getBuffer(), // vpUniformBuffer[i]->getBuffer(), // Buffer get data from
            .offset = 0,                                   // Position of star of data
            .range = sizeof(UboViewProjection)             // Size of data
        };

        // Data about connection between binding and buffer
        const VkWriteDescriptorSet vpSetWrite{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = this->ubo->getDescriptorSets()[i],         // this->descriptorSets->get()[i],            // Descriptor Set to update
            .dstBinding = 0,                                     // Binding to update (matches with binding on layout/shader)
            .dstArrayElement = 0,                                // index in array to update
            .descriptorCount = 1,                                // type of Descriptor
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // Amount to update
            .pBufferInfo = &vpBufferInfo                         // Information about buffer data to bind
        };

        // MODEL DESCRIPTOR
        // Model buffer binding info
        // VkDescriptorBufferInfo modelBufferInfo {
        //     .buffer = this->modelDUniformBuffer[i],
        //     .offset = 0,
        //     .range = this->modelUniformAlignment
        // };

        // // Data about connection between binding and buffer
        // VkWriteDescriptorSet modelSetWrite {
        //     .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        //     .dstSet = this->descriptorSets[i],
        //     .dstBinding = 1,
        //     .dstArrayElement = 0,
        //     .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        //     .descriptorCount = 1,
        //     .pBufferInfo = &modelBufferInfo
        // };

        // List of descriptor set writes
        // std::vector<VkWriteDescriptorSet> setWrites = {vpSetWrite, modelSetWrite};
        this->ubo->addWriteDescriptorSet(vpSetWrite);

        // Update the descripto sets with new buffer/binding info
        this->ubo->updateDescriptorSets();
        this->ubo->clearWriteDescriptorSet();
    }
}

void VulkanRenderer::updateUniformBuffers(uint32_t imageIndex) {

    // Copy VP data
    this->ubo->getUBO()[imageIndex]->mapper(&this->uboViewProjection);

    // // Copy Model data
    // for (size_t i = 0; i < this->meshList.size(); i++) {

    //     std::byte* ptr_base = reinterpret_cast<std::byte*>(this->modelTransferSpace);
    //     std::byte* ptr_atual = ptr_base + (i * this->modelUniformAlignment);
    //     UboModel* thisModel = std::launder(reinterpret_cast<UboModel*>(ptr_atual));

    //     *thisModel = meshList[i].getModel();
    // }

    // // Map the list of model data
    // vkMapMemory(vwrapp->getLogical(), this->modelDUniformBufferMemory[imageIndex], 0, this->modelUniformAlignment * meshList.size(), 0,
    // &data); memcpy(data, this->modelTransferSpace, this->modelUniformAlignment * meshList.size()); vkUnmapMemory(vwrapp->getLogical(),
    // this->modelDUniformBufferMemory[imageIndex]);
}

void VulkanRenderer::recordCommands(uint32_t currentImage) {
    // Information abaout how to begin each command buffer

    // Information about how to begin a render pass (only need for graphical application)
    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {{0.6F, 0.65F, 0.4F, 1.0F}}; // NOLINT(readability-magic-numbers)
    clearValues[1].depthStencil.depth = 1.0F;

    const VkRenderPassBeginInfo renderPassBeginInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = this->rederer->getRenderPass(),                             // Render pass to begin
        .framebuffer = this->swapchain->getSwapChainFrameBuffers()[currentImage], //
        .renderArea = VkRect2D{.offset = {.x = 0, .y = 0},                        // Start point of render pass in pixels
                               .extent = this->swapchain->getExtent()}, // Size of region to run render pass on (starting at offset)
        .clearValueCount = static_cast<uint32_t>(clearValues.size()),   //
        .pClearValues = clearValues.data()                              // List of clear values
    };

    // Start recording command to command buffer!
    // Buffer can be resubmitted when it has alredy been submited and is awaiting execution
    this->commandBuffers->begin(currentImage, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    // Begin Render Pass
    vkCmdBeginRenderPass(this->commandBuffers->getBuffers()[currentImage], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    {
        // Bind Pipeline to be used  in render pass
        vkCmdBindPipeline(this->commandBuffers->getBuffers()[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS,
                          this->pipeline->getGraphicsPipeline());

        for (size_t j = 0; j < this->modelList.size(); j++) { // 1:11:29

            MeshModel thisModel = modelList[j];

            // "Push" constant to given shader stage directly (no buffer)
            vkCmdPushConstants(this->commandBuffers->getBuffers()[currentImage], //
                               this->pipeline->getPipelineLayout(),              //
                               VK_SHADER_STAGE_VERTEX_BIT,                       // Stage to push constant to
                               0,                                                // offset of pushconstant to update
                               sizeof(Model),                                    // size of data being pushed
                               &thisModel.getModel2());                          // Actual data being pushed (cam be array)

            for (size_t k = 0; k < thisModel.getMeshCount(); k++) {
                //

                VkBuffer vertexBuffer[] = {thisModel.getMesh(k)->getVertexBuffer()}; // Buffers to bind
                VkDeviceSize offsets[] = {0};                                        // Offsets into buffers being bound
                vkCmdBindVertexBuffers(commandBuffers->getBuffers()[currentImage], 0, 1, vertexBuffer,
                                       offsets); // Command to bind vertex buffer before drawing with then

                // Bind mesh index buffer, with 0 offset and using the uint32_t type
                vkCmdBindIndexBuffer(commandBuffers->getBuffers()[currentImage], thisModel.getMesh(k)->getIndexBuffer(), 0,
                                     VK_INDEX_TYPE_UINT32);

                // Dynamic offset Amount
                // uint32_t dynamicOffset = static_cast<uint32_t>(this->modelUniformAlignment) * j;

                std::array<VkDescriptorSet, 2> descriptorSetGroup = {this->ubo->getDescriptorSets()[currentImage],
                                                                     this->samplerDescriptorSets->get()[thisModel.getMesh(k)->getTexId()]};

                vkCmdBindDescriptorSets(commandBuffers->getBuffers()[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        this->pipeline->getPipelineLayout(), 0, static_cast<uint32_t>(descriptorSetGroup.size()),
                                        descriptorSetGroup.data(), 0, nullptr);

                // Execute pipeline
                vkCmdDrawIndexed(commandBuffers->getBuffers()[currentImage], thisModel.getMesh(k)->getIndexCount(), 1, 0, 0, 0);
            }
        }
    }
    // End Render Pass
    vkCmdEndRenderPass(this->commandBuffers->getBuffers()[currentImage]);

    this->commandBuffers->end(currentImage);
    //}
}

// void VulkanRenderer::allocateDynamicBufferTransferSpace() {

//     // Caculate alignment of model data
//     this->modelUniformAlignment = (sizeof(UboModel) + this->minUniformBufferOffset - 1) & ~(this->minUniformBufferOffset - 1);

//     // Create space in memory to hold dynamic byffer that is alignment and holds MAX_OBJECTS
//     this->modelTransferSpace = (UboModel*)aligned_alloc(this->modelUniformAlignment, this->modelUniformAlignment * MAX_OBJECTS);
// }

int VulkanRenderer::createTexture(const std::string& filename) {
    // Create Texture image and get its location in array
    int textureImageLoc = this->createTextureImage(filename);
    this->textureImageObjects[textureImageLoc]->createImageView(VK_IMAGE_ASPECT_COLOR_BIT);

    // Create Texture Descriptor
    int descritorLoc = this->createTextureDescriptor(this->textureImageObjects[textureImageLoc]->getImageView());

    // Return location of set with texture
    return descritorLoc;
}

int VulkanRenderer::createTextureImage(const std::string& filename) {
    // Load image
    int width;
    int height;
    VkDeviceSize imageSize;

    stbi_uc* imageData = VulkanRenderer::loadTextureFile(filename, &width, &height, &imageSize);

    // Create staging buffer to hold load data, redy to copy device
    ce::BufferObject imageStagingBuffer(vwrapp->getPhysical(), vwrapp->getLogical());
    imageStagingBuffer.create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // copy image data to staging buffer
    imageStagingBuffer.mapper(imageData);

    // Free original image data
    stbi_image_free(imageData);

    // create image to hold final texture
    std::shared_ptr<ce::ImageObject> texImageObj =
        std::make_shared<ce::ImageObject>(this->vwrapp->getPhysical(), this->vwrapp->getLogical());

    texImageObj->createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // COPY DATA TO IMAGE
    // Transition image to be DST for copy operation
    ce::transitionImageLayout(vwrapp->getLogical(), vwrapp->getGraphicsQueue(), this->graphicsCommandPool->getPool(),
                              texImageObj->getImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy image data
    ce::copyImageBuffer(vwrapp->getLogical(), vwrapp->getGraphicsQueue(), this->graphicsCommandPool->getPool(),
                        imageStagingBuffer.getBuffer(), texImageObj->getImage(), width, height);

    // Transition image to be shader readable for shader
    ce::transitionImageLayout(vwrapp->getLogical(), vwrapp->getGraphicsQueue(), this->graphicsCommandPool->getPool(),
                              texImageObj->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // add texture data to vector for reference
    this->textureImageObjects.push_back(texImageObj);

    return this->textureImageObjects.size() - 1;
}

int VulkanRenderer::createTextureDescriptor(VkImageView textureImage) {
    //
    std::vector<VkDescriptorSetLayout> layouts = {this->samplerSetLayout->get()};
    auto [index, size] = this->samplerDescriptorSets->allocate(this->samplerDescriptorPool->get(), layouts);

    // Texture Image info
    const VkDescriptorImageInfo imageInfo{
        .sampler = this->textureSampler,                        // Image layout when in use
        .imageView = textureImage,                              // Sampler to use for set
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL // Image to bind to set
    };

    // Descriptor Write info
    const VkWriteDescriptorSet descriptorWrite{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                               .dstSet = this->samplerDescriptorSets->get()[index],
                                               .dstBinding = 0,
                                               .dstArrayElement = 0,
                                               .descriptorCount = 1,
                                               .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                               .pImageInfo = &imageInfo};

    // Update new descriptor set
    vkUpdateDescriptorSets(vwrapp->getLogical(), 1, &descriptorWrite, 0, nullptr);

    return this->samplerDescriptorSets->get().size() - 1;
}

int VulkanRenderer::createMeshModel(const std::string& modelFile) {
    // Import model "scene"
    Assimp::Importer importer;

    const aiScene* scene =
        importer.ReadFile(modelFile.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    if (scene == nullptr) {
        throw std::runtime_error("Faile to load model! (" + modelFile + ")");
    }

    // Get vector of all material with 1:1 ID placement
    std::vector<std::string> textureNames = MeshModel::loadMaterials(scene);

    // Convesion from the material list IDs to our Descriptor Array IDs
    std::vector<int> matToTex(textureNames.size());

    // Loop over textureNames and create textures for them
    for (size_t i = 0; i < textureNames.size(); i++) {

        // If material had not texture, set '0' to indicate no texture, texture 0 will be reserved for a default texture
        if (textureNames[i].empty()) { // FIXME: talvez set seria melhor
            matToTex[i] = 0;
        } else {

            // Otherwise, create texture and set value to index of new texture
            matToTex[i] = createTexture(textureNames[i]);
            // matToTex[i] = createTexture("panda.jpg");
        }
    }

    // Load in all our meshes
    std::vector<Mesh> modelMeshes = MeshModel::LoadNode(vwrapp->getPhysical(), vwrapp->getLogical(), vwrapp->getGraphicsQueue(),
                                                        this->graphicsCommandPool->getPool(), scene->mRootNode, scene, matToTex);

    // Create mesh model and add to list
    MeshModel meshModel(modelMeshes);
    this->modelList.push_back(meshModel);

    return this->modelList.size() - 1;
}

stbi_uc* VulkanRenderer::loadTextureFile(const std::string& filename, int* width, int* height, VkDeviceSize* imageSize) {
    // number of chanels image uses
    int channels;

    // Loads pixel data for image
    std::string fileLoc = "./textures/" + filename;
    stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);

    if (image == nullptr) {
        throw std::runtime_error("Failed to load a Texture file  (" + fileLoc + ") !");
    }

    // Calculate image size give a know data
    *imageSize = (*width) * (*height) * 4;

    return image;
}
