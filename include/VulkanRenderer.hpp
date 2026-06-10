#pragma once

#include "MeshModel.hpp"
#include "Renderer.hpp"
#include "SwapChain.hpp"
#include "VWrapp.hpp"
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
    std::shared_ptr<ce::SwapChain> swc;
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
    std::vector<VkFramebuffer> swapChainFrameBuffers;
    std::vector<VkCommandBuffer> commandBuffers;

    VkImage depthBufferImage;
    VkDeviceMemory depthBufferImageMemory;
    VkImageView depthBufferImageView;

    VkSampler textureSampler;

    // - Descriptors
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSetLayout samplerSetLayout;
    VkPushConstantRange pushConstantRange;

    VkDescriptorPool descriptorPool;
    VkDescriptorPool samplerDescriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkDescriptorSet> samplerDescriptorSets;

    std::vector<VkBuffer> vpUniformBuffer;
    std::vector<VkDeviceMemory> vpUniformBufferMemory;

    // std::vector<VkBuffer> modelDUniformBuffer;
    // std::vector<VkDeviceMemory> modelDUniformBufferMemory;

    // VkDeviceSize minUniformBufferOffset;
    // size_t modelUniformAlignment;
    // UboModel* modelTransferSpace;

    // - Assets
    std::vector<VkImage> textureImages;
    std::vector<VkDeviceMemory> textureImageMemory;
    std::vector<VkImageView> textureImageViews;

    // - Pipeline
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;

    // - Pools
    VkCommandPool graphicsCommandPool;

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
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
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
    VkImage createImage(uint32_t with, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags,
                        VkMemoryPropertyFlags propFlags, VkDeviceMemory* imageMemory) const;

    int createTextureImage(const std::string& filename);
    int createTexture(const std::string& filename);
    int createTextureDescriptor(VkImageView textureImage);

    // -- Loader Funcions
    static stbi_uc* loadTextureFile(const std::string& filename, int* width, int* height, VkDeviceSize* imageSize);
};
