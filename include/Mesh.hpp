#pragma once

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "Ultilities.hpp"
#include <GLFW/glfw3.h>
#include <vector>

class Mesh {
  public:
    Mesh();
    Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices);
    ~Mesh();

    int getVertexCount() const;
    VkBuffer getVertexBuffer();

    void destroyVertexBuffer();

  private:
    int vertexCount;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    void createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices);
};
