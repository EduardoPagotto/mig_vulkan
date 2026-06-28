#include "Mesh.hpp"
#include "buffers/utils.hpp"
#include <glm/ext/matrix_float4x4.hpp>

Mesh::Mesh() {
    //
}
Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
           std::vector<Vertex>* vertices, std::vector<uint32_t>* indices, int newTexId) {

    this->physicalDevice = newPhysicalDevice;
    this->device = newDevice;

    this->vertexCount = vertices->size();
    this->createVertexBuffer(transferQueue, transferCommandPool, vertices);

    this->indexCount = indices->size();
    this->createIndexBuffer(transferQueue, transferCommandPool, indices);

    this->model.model = glm::mat4(1.0F);
    this->texId = newTexId;
}
Mesh::~Mesh() {
    //
}

int Mesh::getVertexCount() const { return this->vertexCount; }

VkBuffer Mesh::getVertexBuffer() { return this->vertexBuffer->getBuffer(); }

int Mesh::getIndexCount() const { return this->indexCount; }

VkBuffer Mesh::getIndexBuffer() { return this->indexBuffer->getBuffer(); }

int Mesh::getTexId() const { return this->texId; }

void Mesh::destroyBuffers() {
    this->vertexBuffer.reset();
    this->indexBuffer.reset();
}

void Mesh::createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices) {

    // Get size of buffer needed for vertices
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();

    // Temporary buffer to "stage" vertex data before transfering to GPU
    ce::BufferObject stagingBuffer(physicalDevice, device);

    stagingBuffer.create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer.mapper(vertices->data());

    this->vertexBuffer = std::make_shared<ce::BufferObject>(physicalDevice, device);
    // Create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
    // Buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU(host)
    this->vertexBuffer->create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Copy staging buffer to vertex buffer on GPU
    ce::copyBuffer(this->device, transferQueue, transferCommandPool, stagingBuffer.getBuffer(), this->vertexBuffer->getBuffer(),
                   bufferSize);
}

void Mesh::createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices) {

    // Get size of buffer needed for indices
    VkDeviceSize bufferSize = sizeof(uint32_t) * indices->size();

    // Temporary buffer to "stage" index data before transfering to GPU
    ce::BufferObject stagingBuffer(physicalDevice, device);
    stagingBuffer.create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // MAP MEMORY TO INDEX BUFFER
    stagingBuffer.mapper(indices->data());

    // Create buffer for index data on GPU aceess only area
    this->indexBuffer = std::make_shared<ce::BufferObject>(physicalDevice, device);
    this->indexBuffer->create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Copy from staging buffer to GPU access buffer
    ce::copyBuffer(this->device, transferQueue, transferCommandPool, stagingBuffer.getBuffer(), this->indexBuffer->getBuffer(), bufferSize);
}

void Mesh::setModel(glm::mat4 newModel) { this->model.model = newModel; }

Model Mesh::getModel() { return this->model; }
