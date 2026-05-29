#include "Mesh.hpp"
#include "Ultilities.hpp"
#include <cstring>
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan_core.h>

Mesh::Mesh() {
    //
}
Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices,
           std::vector<uint32_t>* indices) {

    this->physicalDevice = newPhysicalDevice;
    this->device = newDevice;

    this->vertexCount = vertices->size();
    this->createVertexBuffer(transferQueue, transferCommandPool, vertices);

    this->indexCount = indices->size();
    this->createIndexBuffer(transferQueue, transferCommandPool, indices);

    this->uboModel.model = glm::mat4(1.0F);
}
Mesh::~Mesh() {
    //
}

int Mesh::getVertexCount() const { return this->vertexCount; }

VkBuffer Mesh::getVertexBuffer() { return this->vertexBuffer; }

int Mesh::getIndexCount() const { return this->indexCount; }

VkBuffer Mesh::getIndexBuffer() { return this->indexBuffer; }

void Mesh::destroyBuffers() {
    //
    vkDestroyBuffer(this->device, this->vertexBuffer, nullptr);
    vkFreeMemory(this->device, this->vertexBufferMemory, nullptr);
    //
    vkDestroyBuffer(this->device, this->indexBuffer, nullptr);
    vkFreeMemory(this->device, this->indexBufferMemory, nullptr);
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
    copyBuffer(this->device, transferQueue, transferCommandPool, staginBuffer, this->vertexBuffer, bufferSize);

    // Clean up staging buffer parts
    vkDestroyBuffer(this->device, staginBuffer, nullptr);
    vkFreeMemory(this->device, stagingBufferMemory, nullptr);
}

void Mesh::createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices) {

    // Get size of buffer needed for indices
    VkDeviceSize bufferSize = sizeof(uint32_t) * indices->size();

    // Temporary buffer to "stage" index data before transfering to GPU
    VkBuffer staginBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(this->physicalDevice, this->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staginBuffer, &stagingBufferMemory);

    // MAP MEMORY TO INDEX BUFFER
    void* data;                                                              // 1. Create pointer to point in normal memory
    vkMapMemory(this->device, stagingBufferMemory, 0, bufferSize, 0, &data); // 2. "Map" the index buffer memory to that point
    memcpy(data, indices->data(), (size_t)bufferSize);                       // 3. Copy memory from indices vector to the point
    vkUnmapMemory(this->device, stagingBufferMemory);                        // 4. Unmap the vertex buffer memory

    // Create buffer for index data on GPU aceess only area
    createBuffer(this->physicalDevice, this->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &this->indexBuffer, &indexBufferMemory);

    // Copy from staging buffer to GPU access buffer
    copyBuffer(this->device, transferQueue, transferCommandPool, staginBuffer, this->indexBuffer, bufferSize);

    // Destroy + release Staging Buffer resources
    vkDestroyBuffer(this->device, staginBuffer, nullptr);
    vkFreeMemory(this->device, stagingBufferMemory, nullptr);
}

void Mesh::setModel(glm::mat4 newModel) { this->uboModel.model = newModel; }

UboModel Mesh::getModel() { return this->uboModel; }
