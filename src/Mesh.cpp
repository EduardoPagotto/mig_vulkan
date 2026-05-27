#include "Mesh.hpp"
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

Mesh::Mesh() {
    //
}
Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, std::vector<Vertex>* vertices) {
    this->vertexCount = vertices->size();
    this->physicalDevice = newPhysicalDevice;
    this->device = newDevice;
    this->createVertexBuffer(vertices);
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

void Mesh::createVertexBuffer(std::vector<Vertex>* vertices) {

    // Get size of buffer needed for vertices
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();

    // Create Buffer and Allocate Memory to it
    createBuffer(this->physicalDevice, this->device, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &this->vertexBuffer, &this->vertexBufferMemory);

    // MAP MEMORY TO VERTEX BUFFER
    void* data;                                                                   // 1. Create pointer to point in normal memory
    vkMapMemory(this->device, this->vertexBufferMemory, 0, bufferSize, 0, &data); // 2. "Map" the vertex buffer memory to that point
    memcpy(data, vertices->data(), (size_t)bufferSize);                           // 3. Copy memory from vertices vector to the point
    vkUnmapMemory(this->device, this->vertexBufferMemory);                        // 4. Unmap the vertex buffer memory
}
