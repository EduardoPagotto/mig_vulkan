#pragma once

#define GLFW_INCLUDE_VULKAN
#include "Ultilities.hpp"
#include "buffers/IBO.hpp"
#include "buffers/VBO.hpp"
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

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    std::shared_ptr<ce::VBO> vbo;
    std::shared_ptr<ce::IBO> ibo;

    void createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices);
    void createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices);
};
