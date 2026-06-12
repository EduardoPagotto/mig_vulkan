#include "VulkanRenderer.hpp"
#include "DescriptorSetLayout.hpp"
#include "Mesh.hpp"
#include "MeshModel.hpp"
#include "Renderer.hpp"
#include "ShaderModule.hpp"
#include "SwapChain.hpp"
#include "Ultilities.hpp"
#include "VWrapp.hpp"
#include "VWrappUtils.hpp"
#include <array>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
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
#include <vector>
#include <vulkan/vulkan_core.h>

VulkanRenderer::VulkanRenderer(std::shared_ptr<ce::VWrapp> vwrapp) : vwrapp(vwrapp) { // NOLINT

    this->swc = std::make_shared<ce::SwapChain>(vwrapp);         // this->createSwapChain();
    this->rederer = std::make_shared<ce::Renderer>(vwrapp, swc); // this->createRenderPass();
    this->createDescriptorSetLayout();
    this->createPushConstantRange();
    this->createGraphicsPipeline();
    this->createDepthBufferImage();
    this->createFramebuffers();
    this->createCommandPool();
    this->createCommandBuffers();
    this->createTextureSampler();
    // this->allocateDynamicBufferTransferSpace();
    this->createUniformBuffers();
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
    const float aspect = (float)this->swc->getSwapchainExtent().width / (float)this->swc->getSwapchainExtent().height;

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

    for (size_t i = 0; i < this->textureImages.size(); i++) {
        vkDestroyImageView(vwrapp->getLogical(), this->textureImageViews[i], nullptr); // FIXME: ????
        vkDestroyImage(vwrapp->getLogical(), this->textureImages[i], nullptr);
        vkFreeMemory(vwrapp->getLogical(), this->textureImageMemory[i], nullptr);
    }

    vkDestroyImageView(vwrapp->getLogical(), this->depthBufferImageView, nullptr);
    vkDestroyImage(vwrapp->getLogical(), this->depthBufferImage, nullptr);
    vkFreeMemory(vwrapp->getLogical(), this->depthBufferImageMemory, nullptr);

    this->descriptorPool.reset();
    this->descriptorSetLayout.reset();
    for (size_t i = 0; i < this->swc->getSwapchainImages().size(); i++) {
        // destroy uniform
        vkDestroyBuffer(vwrapp->getLogical(), this->vpUniformBuffer[i], nullptr);
        vkFreeMemory(vwrapp->getLogical(), this->vpUniformBufferMemory[i], nullptr);
        // // destroy dynamic uniform
        // vkDestroyBuffer(vwrapp->getLogical(), this->modelDUniformBuffer[i], nullptr);
        // vkFreeMemory(vwrapp->getLogical(), this->modelDUniformBufferMemory[i], nullptr);
    }

    for (size_t i = 0; i < MAX_FRAME_DRAWS; i++) {

        vkDestroySemaphore(vwrapp->getLogical(), this->renderFinished[i], nullptr);
        vkDestroySemaphore(vwrapp->getLogical(), this->imageAvailable[i], nullptr);
        vkDestroyFence(vwrapp->getLogical(), this->drawFences[i], nullptr);
    }

    vkDestroyCommandPool(vwrapp->getLogical(), this->graphicsCommandPool, nullptr);
    for (auto& framebuffer : this->swapChainFrameBuffers) { // ? auto& mesmo ??
        vkDestroyFramebuffer(vwrapp->getLogical(), framebuffer, nullptr);
    }

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
    vkAcquireNextImageKHR(vwrapp->getLogical(), this->swc->getSwapchain(), std::numeric_limits<uint64_t>::max(),
                          this->imageAvailable[this->currentFrame], VK_NULL_HANDLE, &imageIndex);

    this->recordCommands(imageIndex);

    this->updateUniformBuffers(imageIndex);

    // -- SUBMIT COMMAND BUFFER TO RENDER
    // Queue submission information
    VkSemaphore waitSemaphores[] = {this->imageAvailable[this->currentFrame]};
    VkSemaphore signalSemaphores[] = {this->renderFinished[this->currentFrame]};

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;                                                   // Number of semaphores to wait on
    submitInfo.pWaitSemaphores = waitSemaphores;                                         //
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; //
    submitInfo.pWaitDstStageMask = waitStages;                                           // Stagegs to check semaphores at
    submitInfo.commandBufferCount = 1;                                                   // Number of command buffers to submit
    submitInfo.pCommandBuffers = &this->commandBuffers[imageIndex];                      // Command buffer to submit
    submitInfo.signalSemaphoreCount = 1;                                                 // Number of semaphore to signal
    submitInfo.pSignalSemaphores = signalSemaphores;                                     // Semaphore to signal when command buffer finishes

    // Submit command buffer to queue
    VkResult result = vkQueueSubmit(vwrapp->getGraphicsQueue(), 1, &submitInfo, this->drawFences[this->currentFrame]);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit Command Buffer to Queue!");
    }

    // -- PRESENT RENDERED IMAGE TO SCREEN --
    VkPresentInfoKHR presentInfo = {};
    VkSwapchainKHR swapChains[] = {this->swc->getSwapchain()};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;             // Number of semaphores to wait on
    presentInfo.pWaitSemaphores = signalSemaphores; // Semaphores to wait on
    presentInfo.swapchainCount = 1;                 // Number of swapchains to present to
    presentInfo.pSwapchains = swapChains;           // Swapchais to present images to
    presentInfo.pImageIndices = &imageIndex;        // Index of Images in swapchains to present

    // Present Image
    result = vkQueuePresentKHR(vwrapp->getPresentationQueue(), &presentInfo);
    if (result != VK_SUCCESS) {
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
    this->descriptorSetLayout = std::make_shared<ce::DescriptorSetLayout>(this->vwrapp->getLogical());

    // UboViewProjection Binding info
    VkDescriptorSetLayoutBinding vpLayoutBinding = {};
    vpLayoutBinding.binding = 0;                                        // Binding point in shader (designed by binding number in shader)
    vpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Type of descriptor (uniform, dynamic, image sampler, etc)
    vpLayoutBinding.descriptorCount = 1;                                // Number of descriptors for binding
    vpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;            // Shade stage to bind to
    vpLayoutBinding.pImmutableSamplers = nullptr; // for Texture: can make sampler unchangeable (immutable) by specifying in layout
    this->descriptorSetLayout->addBinding(vpLayoutBinding);

    // Model Binding Info
    // VkDescriptorSetLayoutBinding mLayoutBinding = {};
    // mLayoutBinding.binding = 1;
    // mLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    // mLayoutBinding.descriptorCount = 1;
    // mLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    // mLayoutBinding.pImmutableSamplers = nullptr;
    // this->descriptorSetLayout->addBinding(mLayoutBinding);

    this->descriptorSetLayout->create();

    // CREATE TEXTURE SAMPLER DESCRIPTOR SET LAYOUT
    this->samplerSetLayout = std::make_shared<ce::DescriptorSetLayout>(this->vwrapp->getLogical());
    // Texture binding info
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    this->samplerSetLayout->addBinding(samplerLayoutBinding);
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
    auto viewport = VkViewport{.x = 0.0F,                                               // x start coordinate
                               .y = 0.0F,                                               // y start coordinate
                               .width = (float)this->swc->getSwapchainExtent().width,   // width of viewport
                               .height = (float)this->swc->getSwapchainExtent().height, // height of viewport
                               .minDepth = 0.0F,                                        // min framebuffer depth
                               .maxDepth = 1.0F};                                       // max framebuffer depth

    auto scissor = VkRect2D{.offset = VkOffset2D{.x = 0, .y = 0},       // Offset to use region from
                            .extent = this->swc->getSwapchainExtent()}; // Extent to describe region to use, starting at offset

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
    VkPipelineColorBlendAttachmentState colourState = {};
    colourState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                 VK_COLOR_COMPONENT_A_BIT; // Color to apply blending to
    colourState.blendEnable = VK_TRUE;                     // Enable blending

    // Blending uses equation: (srcColorBlendfactor * new colour) colorBlendOp (dstColorBlendfactor * old colour)
    colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colourState.colorBlendOp = VK_BLEND_OP_ADD;

    // Sumarised: (VK_BLEND_FACTOR_SRC_ALPHA * new colour) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old colour)
    //            (new colour alpha * new colour) + ((i - new color alpha) * old colour)

    colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colourState.alphaBlendOp = VK_BLEND_OP_ADD;
    // Sumarized: (1 * new alpha) + (0 * old Alpha) = new alpha

    this->pipeline->addColourState(colourState);

    // -- PIPELINE LAYOUT --
    this->pipeline->addLayout(this->descriptorSetLayout->getDescriptorSetLayout());
    this->pipeline->addLayout(this->samplerSetLayout->getDescriptorSetLayout());
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
    this->depthBufferImage =
        ce::createImage(this->vwrapp->getPhysical(), this->vwrapp->getLogical(), this->swc->getSwapchainExtent().width,
                        this->swc->getSwapchainExtent().height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &this->depthBufferImageMemory);

    // Create Depth Buffer Image View
    this->depthBufferImageView =
        ce::CreateImageView(this->vwrapp->getLogical(), this->depthBufferImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanRenderer::createFramebuffers() {
    // Resize framebuffer count to equal chain image count
    this->swapChainFrameBuffers.resize(this->swc->getSwapchainImages().size());

    // Create a framebuffer for eache swap chain image
    for (size_t i = 0; i < this->swapChainFrameBuffers.size(); i++) {

        std::array<VkImageView, 2> attachments = {this->swc->getSwapchainImages()[i].imageView,
                                                  this->depthBufferImageView}; // order important same as upper

        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = this->rederer->getRenderPass(); // Render Pass layout the framebuffer will be used with
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferCreateInfo.pAttachments = attachments.data();               // List of attachments (1:1 with Render Pass)
        framebufferCreateInfo.width = this->swc->getSwapchainExtent().width;   // Framebuffer width
        framebufferCreateInfo.height = this->swc->getSwapchainExtent().height; // Framebuffer height
        framebufferCreateInfo.layers = 1;                                      // Framebuffer layers

        VkResult result = vkCreateFramebuffer(vwrapp->getLogical(), &framebufferCreateInfo, nullptr, &this->swapChainFrameBuffers[i]);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Faleid to create a frambuffer");
        }
    }
}

void VulkanRenderer::createCommandPool() {

    // Get inidices of queue families from device
    ce::QueueFamilyIndices queueFamilyIndices = ce::GetQueueFamilies(vwrapp->getPhysical(), vwrapp->getSurface());

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily; // Queue Family type that buffers from this command pool will use

    // Create a Graphics Queue Family Command Pool
    VkResult result = vkCreateCommandPool(vwrapp->getLogical(), &poolInfo, nullptr, &this->graphicsCommandPool);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Commnad Pool");
    }
}

void VulkanRenderer::createCommandBuffers() {

    // Resize command buffer count to have one for each frambuffer
    this->commandBuffers.resize(this->swapChainFrameBuffers.size());

    VkCommandBufferAllocateInfo cbAllocInfo = {};
    cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbAllocInfo.commandPool = graphicsCommandPool;
    cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // VK_COMMAND_BUFFER_LEVEL_PRIMARY : Buffer you submit directly
                                                         // to queue. Can't be called by other buffers.
                                                         // VK_COMMAND_BUFFER_LEVEL_SECUNDARY : Buffer can't be called
                                                         // directly. cam be called from other buffe via
                                                         // "VkCmdExecuteCommand" when recording commands in primary buf
    cbAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    // Allocate command buffers and place handles in array of buffers
    VkResult result = vkAllocateCommandBuffers(vwrapp->getLogical(), &cbAllocInfo, this->commandBuffers.data());

    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to Allocate Command buffers!");
    }
}

void VulkanRenderer::createSynchronisation() {

    this->imageAvailable.resize(MAX_FRAME_DRAWS);
    this->renderFinished.resize(MAX_FRAME_DRAWS);
    this->drawFences.resize(MAX_FRAME_DRAWS);

    // Semaphore creation information
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // Fence creation information
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

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
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;                   // How torender when image is magnified on screen
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;                   // How to render when image is minifield on screen
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;  // How to handle texture wrap in U(x) direction
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;  // How to handle texture wrap in V(y) direction
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;  // How to handle texture wrap in W(z) direction
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Border beond texture (only works for border clamp)
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;             // Wheter coords should be normalized (between 0 and 1)
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;     // Mipmap interpolation mode
    samplerCreateInfo.mipLodBias = 0.0F;                              // Level of detail of bias for mip level
    samplerCreateInfo.minLod = 0.0F;                                  // Minimum Level Detail ro pick mip level
    samplerCreateInfo.maxLod = 0.0F;                                  // Maximum Level Detail ro pick mip level
    samplerCreateInfo.anisotropyEnable = VK_TRUE;                     // Enable anisotropy
    samplerCreateInfo.maxAnisotropy = 16;                             // Anisotropy sample level

    VkResult result = vkCreateSampler(vwrapp->getLogical(), &samplerCreateInfo, nullptr, &this->textureSampler);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a Sampler");
    }
}

void VulkanRenderer::createUniformBuffers() {

    // ViewProjection Buffer size
    VkDeviceSize vpBufferSize = sizeof(UboViewProjection);

    // // Model Buffer size
    // VkDeviceSize modelBufferSize = this->modelUniformAlignment * MAX_OBJECTS;

    // One uniform buffer for each image (and by extention, command buffer)
    this->vpUniformBuffer.resize(this->swc->getSwapchainImages().size());
    this->vpUniformBufferMemory.resize(this->swc->getSwapchainImages().size());

    // this->modelDUniformBuffer.resize(this->swc->getSwapchainImages().size());
    // this->modelDUniformBufferMemory.resize(this->swc->getSwapchainImages().size());

    // Create Unifor buffers
    for (size_t i = 0; i < this->swc->getSwapchainImages().size(); i++) {
        ce::createBuffer(vwrapp->getPhysical(), vwrapp->getLogical(), vpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &this->vpUniformBuffer[i],
                         &this->vpUniformBufferMemory[i]);

        // createBuffer(vwrapp->getPhysical(), vwrapp->getLogical(), modelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        //              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &this->modelDUniformBuffer[i],
        //              &this->modelDUniformBufferMemory[i]);
    }
}

void VulkanRenderer::createDescriptorPool() {

    // CREATE DESCRIPTOR POOL
    // CREATE UNIFORM DESCRIPTOR POOL
    this->descriptorPool = std::make_shared<ce::DescriptorPool>(this->vwrapp->getLogical());
    // Type of Descriptors + how many DESCRIPTORS, not Descriptor Sets (combined makes the pool size)
    // ViewProjection Pool
    this->descriptorPool->addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(this->vpUniformBuffer.size()));

    // // Model Pool (Dynamic)
    // this->descriptorPool->addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, //
    //                                    static_cast<uint32_t>(this->modelDUniformBuffer.size());//

    // Create Descriptor Pool
    this->descriptorPool->create(static_cast<uint32_t>(this->swc->getSwapchainImages().size())); // Maximum number of descriptor Sets
    ;

    // -- CREATE UNIFORM DESCRIPTOR POOL
    // Texture sampler pool
    this->samplerDescriptorPool = std::make_shared<ce::DescriptorPool>(this->vwrapp->getLogical());
    this->samplerDescriptorPool->addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_OBJECTS);
    this->samplerDescriptorPool->create(MAX_OBJECTS);
}

void VulkanRenderer::createDescriptorSets() {
    // Resize Descriptor Set list so one for every buffer
    this->descriptorSets.resize(this->swc->getSwapchainImages().size());

    std::vector<VkDescriptorSetLayout> setLayouts(this->swc->getSwapchainImages().size(),
                                                  this->descriptorSetLayout->getDescriptorSetLayout());

    // Descriptor set allocation info
    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = this->descriptorPool->getDescriptorPool();                         // Pool to allocate Descriptor Set from
    setAllocInfo.descriptorSetCount = static_cast<uint32_t>(this->swc->getSwapchainImages().size()); // Number of sets to allocate
    setAllocInfo.pSetLayouts = setLayouts.data(); // Layouts to use to allocate sets (1:1 relationship)

    // Allocate descriptor sets (multiple)
    VkResult result = vkAllocateDescriptorSets(vwrapp->getLogical(), &setAllocInfo, this->descriptorSets.data());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to Allocate Descriptor sets!");
    }

    // Update all of descriptor set buffer bindings
    for (size_t i = 0; i < this->swc->getSwapchainImages().size(); i++) {

        // VIEW PROJECTION DESCRIPTOR
        // Buffer info and data offset info
        VkDescriptorBufferInfo vpBufferInfo = {};
        vpBufferInfo.buffer = this->vpUniformBuffer[i]; // Buffer get data from
        vpBufferInfo.offset = 0;                        // Position of star of data
        vpBufferInfo.range = sizeof(UboViewProjection); // Size of data

        // Data about connection between binding and buffer
        VkWriteDescriptorSet vpSetWrite = {};
        vpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vpSetWrite.dstSet = this->descriptorSets[i];                   // Descriptor Set to update
        vpSetWrite.dstBinding = 0;                                     // Binding to update (matches with binding on layout/shader)
        vpSetWrite.dstArrayElement = 0;                                // index in array to update
        vpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of Descriptor
        vpSetWrite.descriptorCount = 1;                                // Amount to update
        vpSetWrite.pBufferInfo = &vpBufferInfo;                        // Information about buffer data to bind

        // // MODEL DESCRIPTOR
        // // Model buffer binding info
        // VkDescriptorBufferInfo modelBufferInfo = {};
        // modelBufferInfo.buffer = this->modelDUniformBuffer[i];
        // modelBufferInfo.offset = 0;
        // modelBufferInfo.range = this->modelUniformAlignment;

        // // Data about connection between binding and buffer
        // VkWriteDescriptorSet modelSetWrite = {};
        // modelSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // modelSetWrite.dstSet = this->descriptorSets[i];
        // modelSetWrite.dstBinding = 1;
        // modelSetWrite.dstArrayElement = 0;
        // modelSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        // modelSetWrite.descriptorCount = 1;
        // modelSetWrite.pBufferInfo = &modelBufferInfo;

        // List of descriptor set writes
        // std::vector<VkWriteDescriptorSet> setWrites = {vpSetWrite, modelSetWrite};
        std::vector<VkWriteDescriptorSet> setWrites = {vpSetWrite};

        // Update the descripto sets with new buffer/binding info
        vkUpdateDescriptorSets(vwrapp->getLogical(), static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
    }
}

void VulkanRenderer::updateUniformBuffers(uint32_t imageIndex) {

    // Copy VP data
    void* data;
    vkMapMemory(vwrapp->getLogical(), this->vpUniformBufferMemory[imageIndex], 0, sizeof(UboViewProjection), 0, &data);
    memcpy(data, &this->uboViewProjection, sizeof(UboViewProjection));
    vkUnmapMemory(vwrapp->getLogical(), this->vpUniformBufferMemory[imageIndex]);

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

    VkCommandBufferBeginInfo bufferBeginInfo = {};
    bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bufferBeginInfo.flags =
        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Buffer can be resubmitted when it has alredy been submited and is awaiting
    // execution

    // Information about how to begin a render pass (only need for graphical application)
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = this->rederer->getRenderPass();
    ;                                                                        // Render pass to begin
    renderPassBeginInfo.renderArea.offset = {.x = 0, .y = 0};                // Start point of render pass in pixels
    renderPassBeginInfo.renderArea.extent = this->swc->getSwapchainExtent(); // Size of region to run render pass on (starting at offset)

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {{0.6F, 0.65F, 0.4F, 1.0F}}; // NOLINT(readability-magic-numbers)
    clearValues[1].depthStencil.depth = 1.0F;

    renderPassBeginInfo.pClearValues = clearValues.data(); // List of clear values
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

    // for (size_t i = 0; i < this->commandBuffers.size(); i++) {

    renderPassBeginInfo.framebuffer = this->swapChainFrameBuffers[currentImage];

    // Start recording command to command buffer!
    VkResult result = vkBeginCommandBuffer(this->commandBuffers[currentImage], &bufferBeginInfo);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to start recording a Command Buffer!");
    }

    // Begin Render Pass
    vkCmdBeginRenderPass(this->commandBuffers[currentImage], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    {
        // Bind Pipeline to be used  in render pass
        vkCmdBindPipeline(this->commandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->getGraphicsPipeline());

        for (size_t j = 0; j < this->modelList.size(); j++) { // 1:11:29

            MeshModel thisModel = modelList[j];

            // "Push" constant to given shader stage directly (no buffer)
            vkCmdPushConstants(this->commandBuffers[currentImage],  //
                               this->pipeline->getPipelineLayout(), //
                               VK_SHADER_STAGE_VERTEX_BIT,          // Stage to push constant to
                               0,                                   // offset of pushconstant to update
                               sizeof(Model),                       // size of data being pushed
                               &thisModel.getModel2());             // Actual data being pushed (cam be array)

            for (size_t k = 0; k < thisModel.getMeshCount(); k++) {
                //

                VkBuffer vertexBuffer[] = {thisModel.getMesh(k)->getVertexBuffer()}; // Buffers to bind
                VkDeviceSize offsets[] = {0};                                        // Offsets into buffers being bound
                vkCmdBindVertexBuffers(commandBuffers[currentImage], 0, 1, vertexBuffer,
                                       offsets); // Command to bind vertex buffer before drawing with then

                // Bind mesh index buffer, with 0 offset and using the uint32_t type
                vkCmdBindIndexBuffer(commandBuffers[currentImage], thisModel.getMesh(k)->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

                // Dynamic offset Amount
                // uint32_t dynamicOffset = static_cast<uint32_t>(this->modelUniformAlignment) * j;

                std::array<VkDescriptorSet, 2> descriptorSetGroup = {this->descriptorSets[currentImage],
                                                                     this->samplerDescriptorSets[thisModel.getMesh(k)->getTexId()]};

                vkCmdBindDescriptorSets(commandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->getPipelineLayout(),
                                        0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);

                // Execute pipeline
                vkCmdDrawIndexed(commandBuffers[currentImage], thisModel.getMesh(k)->getIndexCount(), 1, 0, 0, 0);
            }
        }
    }
    // End Render Pass
    vkCmdEndRenderPass(this->commandBuffers[currentImage]);

    result = vkEndCommandBuffer(this->commandBuffers[currentImage]);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to stop recording a Command Buffer!");
    }
    //}
}

// void VulkanRenderer::allocateDynamicBufferTransferSpace() {

//     // Caculate alignment of model data
//     this->modelUniformAlignment = (sizeof(UboModel) + this->minUniformBufferOffset - 1) & ~(this->minUniformBufferOffset - 1);

//     // Create space in memory to hold dynamic byffer that is alignment and holds MAX_OBJECTS
//     this->modelTransferSpace = (UboModel*)aligned_alloc(this->modelUniformAlignment, this->modelUniformAlignment * MAX_OBJECTS);
// }

int VulkanRenderer::createTexture(const std::string& filename) {
    //

    // Create Texture image and get its location in array
    int textureImageLoc = this->createTextureImage(filename);

    // Create image view and add list
    VkImageView imageView = ce::CreateImageView(this->vwrapp->getLogical(), this->textureImages[textureImageLoc], VK_FORMAT_R8G8B8A8_UNORM,
                                                VK_IMAGE_ASPECT_COLOR_BIT);
    textureImageViews.push_back(imageView);

    // TCreate Texture Descriptor
    int descritorLoc = this->createTextureDescriptor(imageView);

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
    VkBuffer imageStagingBuffer;
    VkDeviceMemory imageStagingBufferMemory;
    ce::createBuffer(vwrapp->getPhysical(), vwrapp->getLogical(), imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &imageStagingBuffer,
                     &imageStagingBufferMemory);

    // copy image data to staging buffer
    void* data;
    vkMapMemory(vwrapp->getLogical(), imageStagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, imageData, static_cast<size_t>(imageSize));
    vkUnmapMemory(vwrapp->getLogical(), imageStagingBufferMemory);

    // Free original image data
    stbi_image_free(imageData);

    // create image to hold final texture
    VkImage texImage;
    VkDeviceMemory texImageMemory;
    texImage = ce::createImage(this->vwrapp->getPhysical(), this->vwrapp->getLogical(), width, height, VK_FORMAT_R8G8B8A8_UNORM,
                               VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory);

    // COPY DATA TO IMAGE
    // Transition image to be DST for copy operation
    transitionImageLayout(vwrapp->getLogical(), vwrapp->getGraphicsQueue(), graphicsCommandPool, texImage, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy image data
    copyImageBuffer(vwrapp->getLogical(), vwrapp->getGraphicsQueue(), graphicsCommandPool, imageStagingBuffer, texImage, width, height);

    // Transition image to be shader readable for shader
    transitionImageLayout(vwrapp->getLogical(), vwrapp->getGraphicsQueue(), graphicsCommandPool, texImage,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // add texture data to vector for reference
    this->textureImages.push_back(texImage);
    this->textureImageMemory.push_back(texImageMemory);

    // Destroy staging buffers
    vkDestroyBuffer(vwrapp->getLogical(), imageStagingBuffer, nullptr);
    vkFreeMemory(vwrapp->getLogical(), imageStagingBufferMemory, nullptr);

    // Return index of new texture image
    return this->textureImages.size() - 1;
}

int VulkanRenderer::createTextureDescriptor(VkImageView textureImage) {
    //
    VkDescriptorSet descriptorSet;

    // Descriptor Set Allocation info
    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = this->samplerDescriptorPool->getDescriptorPool(); // this->samplerDescriptorPool;
    setAllocInfo.descriptorSetCount = 1;
    setAllocInfo.pSetLayouts = &this->samplerSetLayout->getDescriptorSetLayout(); // samplerSetLayout;

    // Allocate descriptr sets
    VkResult result = vkAllocateDescriptorSets(vwrapp->getLogical(), &setAllocInfo, &descriptorSet);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate Texture descriptor set");
    }

    // Texture Image info
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // Image layout when in use
    imageInfo.imageView = textureImage;                               // Image to bind to set
    imageInfo.sampler = this->textureSampler;                         // Sampler to use for set

    // Descriptor Write info
    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    // Update new descriptor set
    vkUpdateDescriptorSets(vwrapp->getLogical(), 1, &descriptorWrite, 0, nullptr);

    // Add descriptor set to list
    samplerDescriptorSets.push_back(descriptorSet);

    return samplerDescriptorSets.size() - 1;
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
                                                        this->graphicsCommandPool, scene->mRootNode, scene, matToTex);

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
