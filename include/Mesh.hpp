#pragma once

#define GLFW_INCLUDE_VULKAN
#include "Ultilities.hpp"
#include "buffers/BufferObject.hpp"
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

struct Model {
    glm::mat4 model;
};

class Mesh {
  public:
    Mesh();
    Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
         std::vector<Vertex>* vertices, std::vector<uint32_t>* indices, int newTexId);
    ~Mesh();

    void setModel(glm::mat4 newModel);
    Model getModel();

    int getTexId() const;

    const Model& getModel2() const { return this->model; }

    int getVertexCount() const;
    VkBuffer getVertexBuffer();

    int getIndexCount() const;
    VkBuffer getIndexBuffer();

    void destroyBuffers();

  private:
    Model model;

    int texId;

    int vertexCount;
    std::shared_ptr<ce::BufferObject> vertexBuffer;

    int indexCount;
    std::shared_ptr<ce::BufferObject> indexBuffer;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    void createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices);
    void createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices);
};
