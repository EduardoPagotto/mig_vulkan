#include "Mesh.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

Mesh::Mesh() {
    //
}
Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, std::vector<Vertex>* vertices) {
    this->vertexCount = vertices->size();
    this->physicalDevice = newPhysicalDevice;
    this->device = newDevice;
    this->vertexBuffer = this->createVertexBuffer(vertices);
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
    // parei em: 55:26
}

VkBuffer Mesh::createVertexBuffer(std::vector<Vertex>* vertices) {
    // information to create a buffer (dosen't include assigning memory)
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Vertex) * vertices->size();  // Size of buffer (size of 1 vertex * number of vertices)
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; // Multiple types of buffer possible, we want Vertex Buffer
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;   // Similar to Swap Chain images, can share vertex buffers

    VkResult result = vkCreateBuffer(this->device, &bufferInfo, nullptr, &this->vertexBuffer);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to creaste a Vertex Buffer!");
    }

    // GET BUFFER MEMORY REQUIREMENTS
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(this->device, this->vertexBuffer, &memRequirements);

    // ALLOCATE MEMORY TO BUFFER
    VkMemoryAllocateInfo memoryAllocInfo = {};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memRequirements.size;
    memoryAllocInfo.memoryTypeIndex =
        this->findMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    ; // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : CPU can interact with memory
    ; // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : Allows placement of data straight into buffer mapping (otherwise would have to specify manually)

    // Allocate memory to VkDebviceMemory
    result = vkAllocateMemory(this->device, &memoryAllocInfo, nullptr, &this->vertexBufferMemory);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate Vertex Buffer Memory!!");
    }

    // Allocate memory to given vertex buffer
    vkBindBufferMemory(this->device, this->vertexBuffer, this->vertexBufferMemory, 0);
}

uint32_t Mesh::findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags properties) {
    // get properties of physical device memory
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

        // Index of memory type must match corresponding bit in allowedTypes and desired property bit flag are part of memory type's property flags
        if (((allowedTypes & (1 << i))) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            // this memory type is valid, so return its index
            return i;
        }
    }
}
