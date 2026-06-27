#pragma once

#include "BufferObject.hpp"
#include "Command.hpp"
#include "ImageObject.hpp"
#include "MeshModel.hpp"
#include "Pipeline.hpp"
#include "Renderer.hpp"
#include "SwapChain.hpp"
#include "VWrapp.hpp"
#include "descriptors/DescriptorPool.hpp"
#include "descriptors/DescriptorSet.hpp"
#include "descriptors/DescriptorSetLayout.hpp"
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

class VulkanRenderer {
  public:
    explicit VulkanRenderer(std::shared_ptr<ce::VWrapp> vwrapp);

    virtual ~VulkanRenderer();

    void updateModel(int modelId, glm::mat4 newModel);

    int createMeshModel(const std::string& modelFile);

    void draw();

  private:
    int currentFrame = 0;

    std::shared_ptr<ce::VWrapp> vwrapp;
    std::shared_ptr<ce::SwapChain> swapchain;
    std::shared_ptr<ce::Renderer> rederer;

    // Scene Objects
    std::vector<MeshModel> modelList;

    // Scene Settings
    struct UboViewProjection {
        glm::mat4 projection;
        glm::mat4 view;
    } uboViewProjection;

    // Vulkan components
    // - Main
    std::shared_ptr<ce::CommandBuffer> commandBuffers;
    std::shared_ptr<ce::ImageObject> depthBufferObject;

    VkSampler textureSampler; // TODO: aqui!!! depois o draw!!!

    // - Descriptors
    std::shared_ptr<ce::DescriptorSetLayout> descriptorSetLayout;
    std::shared_ptr<ce::DescriptorSetLayout> samplerSetLayout;

    VkPushConstantRange pushConstantRange;

    std::shared_ptr<ce::DescriptorPool> descriptorPool;
    std::shared_ptr<ce::DescriptorPool> samplerDescriptorPool;
    std::shared_ptr<ce::DescriptorSet> descriptorSets;
    std::shared_ptr<ce::DescriptorSet> samplerDescriptorSets;

    std::vector<std::shared_ptr<ce::BufferObject>> vpUniformBuffer;

    // std::vector<VkBuffer> modelDUniformBuffer;
    // std::vector<VkDeviceMemory> modelDUniformBufferMemory;

    // VkDeviceSize minUniformBufferOffset;
    // size_t modelUniformAlignment;
    // UboModel* modelTransferSpace;

    // - Assets
    std::vector<std::shared_ptr<ce::ImageObject>> textureImageObjects;

    // - Pipeline
    std::shared_ptr<ce::Pipeline> pipeline;

    // - Pools
    std::shared_ptr<ce::CommandPool> graphicsCommandPool;

    // - Synchronization
    std::vector<VkSemaphore> imageAvailable;
    std::vector<VkSemaphore> renderFinished;
    std::vector<VkFence> drawFences;

    // Vulkan Functions
    // - Create functions
    void createDescriptorSetLayout();
    void createPushConstantRange();
    void createGraphicsPipeline();
    void createDepthBufferImage();
    void createCommandPool();
    void createSynchronisation();
    void createTextureSampler();

    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();

    void updateUniformBuffers(uint32_t imageIndex);

    // - Record Functions
    void recordCommands(uint32_t currentImage);

    // - Allocate functions
    // void allocateDynamicBufferTransferSpace();

    // -- Create Functions
    int createTextureImage(const std::string& filename);
    int createTexture(const std::string& filename);
    int createTextureDescriptor(VkImageView textureImage);

    // -- Loader Funcions
    static stbi_uc* loadTextureFile(const std::string& filename, int* width, int* height, VkDeviceSize* imageSize);
};
