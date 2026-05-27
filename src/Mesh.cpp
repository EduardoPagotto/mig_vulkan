#include "Mesh.hpp"
#include "Ultilities.hpp"
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

Mesh::Mesh() {
    //
}
Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices) {
    this->vertexCount = vertices->size();
    this->physicalDevice = newPhysicalDevice;
    this->device = newDevice;
    this->createVertexBuffer(transferQueue, transferCommandPool, vertices);
}
Mesh::~Mesh() {
    //
}
int Mesh::getVertexCount() const { return this->vertexCount; }

VkBuffer Mesh::getVertexBuffer() { return this->vertexBuffer; }

void Mesh::destroyVertexBuffer() {
    //
    vkDestroyBuffer(this->device, this->vertexBuffer, nullptr);
    vkFreeMemory(this->device, this->vertexBufferMemory, nullptr);
}

void Mesh::createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices) {

    // Get size of buffer needed for vertices
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();

    // Temporary buffer to "stage" vertex data before transfering to GPU
    VkBuffer staginBuffer;
    VkDeviceMemory stagingBufferMemory;

    // Create Staging Buffer and Allocate Memory to it
    createBuffer(this->physicalDevice, this->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staginBuffer, &stagingBufferMemory);

    // MAP MEMORY TO VERTEX BUFFER
    void* data;                                                              // 1. Create pointer to point in normal memory
    vkMapMemory(this->device, stagingBufferMemory, 0, bufferSize, 0, &data); // 2. "Map" the vertex buffer memory to that point
    memcpy(data, vertices->data(), (size_t)bufferSize);                      // 3. Copy memory from vertices vector to the point
    vkUnmapMemory(this->device, stagingBufferMemory);                        // 4. Unmap the vertex buffer memory

    // Create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
    // Buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU(host)
    createBuffer(this->physicalDevice, this->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &this->vertexBuffer, &this->vertexBufferMemory);

    // Copy staging buffer to vertex buffer on GPU
    copyBuffer(this->device, transferQueue, transferCommandPool, staginBuffer, vertexBuffer, bufferSize);

    // Clean up staging buffer parts
    vkDestroyBuffer(this->device, staginBuffer, nullptr);
    vkFreeMemory(this->device, stagingBufferMemory, nullptr);
}
