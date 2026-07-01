#pragma once

#include "Command.hpp"
#include "Pipeline.hpp"
#include "Renderer.hpp"
#include "SwapChain.hpp"
#include "Textures.hpp"
#include "buffers/CommandBuffer.hpp"
#include "buffers/ImageObject.hpp"
#include "buffers/UBO.hpp"
#include "entitys/MeshModel.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

class VulkanRenderer {
  public:
    explicit VulkanRenderer(ce::DevVk& devvk);
    virtual ~VulkanRenderer();

    void updateModel(int modelId, glm::mat4 newModel);
    int createMeshModel(const std::string& modelFile);
    void draw();

  private:
    int currentFrame = 0;

    VkPhysicalDevice physical;
    VkDevice logical;
    VkQueue gQueue; // graphicsQueue
    VkQueue pQueue; // presentationQueue
    VkSurfaceKHR surface;

    // std::shared_ptr<ce::VWrapp> vwrapp;
    std::shared_ptr<ce::SwapChain> swapchain;
    std::shared_ptr<ce::Renderer> rederer;

    // Scene Objects
    std::vector<ce::MeshModel> modelList;

    // Scene Settings
    struct UboViewProjection {
        glm::mat4 projection;
        glm::mat4 view;
    } uboViewProjection;

    // Vulkan components
    // - Main
    std::shared_ptr<ce::CommandBuffer> commandBuffers;
    std::shared_ptr<ce::ImageObject> depthBufferObject;

    // - Descriptors
    std::shared_ptr<ce::DescriptorPool> descriptorPool;
    std::shared_ptr<ce::UBO<ce::BufferObject>> uboVP;

    VkPushConstantRange pushConstantRange;

    // - Assets
    std::shared_ptr<ce::Textures> textureMng;

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
    void createDescriptorPool();
    void createDescriptorSets();

    void updateUniformBuffers(uint32_t imageIndex);

    // - Record Functions
    void recordCommands(uint32_t currentImage);
};
