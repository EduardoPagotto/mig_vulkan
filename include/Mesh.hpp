#pragma once

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "Ultilities.hpp"
#include <GLFW/glfw3.h>
#include <vector>

struct Model {
    glm::mat4 model;
};

class Mesh {
  public:
    Mesh();
    Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices,
         std::vector<uint32_t>* indices);
    ~Mesh();

    void setModel(glm::mat4 newModel);
    Model getModel();
    const Model& getModel2() const { return this->model; }

    int getVertexCount() const;
    VkBuffer getVertexBuffer();

    int getIndexCount() const;
    VkBuffer getIndexBuffer();

    void destroyBuffers();

  private:
    Model model;

    int vertexCount;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    int indexCount;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    void createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices);
    void createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices);
};
