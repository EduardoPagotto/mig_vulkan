#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstddef>
#include <fstream>
#include <glm/glm.hpp>
#include <ios>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

const int MAX_FRAME_DRAWS = 2;
const int MAX_OBJECTS = 2;

const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

// Vertex data representation
struct Vertex {
    glm::vec3 pos; // Vertex Position (x, y, z)
    glm::vec3 col; // Vertex Color (r, g, b)
};

struct QueueFamilyIndices {
    int graphicsFamily = -1;     // Location of graphics Queue Family
    int presentationFamily = -1; // Location of Presentation Queue family

    // check if queue families are valid
    [[nodiscard]] bool isValid() const { return (graphicsFamily >= 0) && (presentationFamily >= 0); }
};

struct SwapChainDetails {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;    // Surface properties, e.g. image size/extent
    std::vector<VkSurfaceFormatKHR> formats;         // Surface image formats, e.g. RGBA and size of each colour
    std::vector<VkPresentModeKHR> presentationModes; // How images should be presented to screen
};

struct SwapchainImage {
    VkImage image;
    VkImageView imageView;
};

[[maybe_unused]] static std::vector<char> readFile(const std::string& filename) {
    // Open stream from given file
    // std::ios::binary tells stream to read file as binary
    // std::ios::ate tells stream to start reading from end file
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    // Chack if fstream sucessfully open
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open a file!");
    }

    size_t filesize = (size_t)file.tellg();

    std::vector<char> fileBuffer(filesize);

    // Move read position (seek to0 the start of the file)
    file.seekg(0);

    // Read the file data into the buffer (stream "fileSize" in total)
    file.read(fileBuffer.data(), filesize);

    // Close stream
    file.close();

    return fileBuffer;
}

static uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties) {
    // get properties of physical device memory
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

        // Index of memory type must match corresponding bit in allowedTypes and desired property bit flag are part of memory type's property flags
        if ((allowedTypes & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            // this memory type is valid, so return its index
            return i;
        }
    }

    throw std::runtime_error("Failed to find Memory!");
}

[[maybe_unused]] static void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage,
                                          VkMemoryPropertyFlags bufferProperties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
    // CREATE VERTEX BUFFER
    // information to create a buffer (dosen't include assigning memory)
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;                       // Size of buffer (size of 1 vertex * number of vertices)
    bufferInfo.usage = bufferUsage;                     // Multiple types of buffer possible
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Similar to Swap Chain images, can share vertex buffers

    VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, buffer);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to creaste a Vertex Buffer!");
    }

    // GET BUFFER MEMORY REQUIREMENTS
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    // ALLOCATE MEMORY TO BUFFER
    VkMemoryAllocateInfo memoryAllocInfo = {};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memRequirements.size;
    memoryAllocInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memRequirements.memoryTypeBits, bufferProperties);
    ; // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : CPU can interact with memory
    ; // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : Allows placement of data straight into buffer mapping (otherwise would have to specify manually)

    // Allocate memory to VkDebviceMemory
    result = vkAllocateMemory(device, &memoryAllocInfo, nullptr, bufferMemory);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate Vertex Buffer Memory!!");
    }

    // Allocate memory to given vertex buffer
    vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

static VkCommandBuffer beginCommandBuffer(VkDevice device, VkCommandPool commandPool) {
    // Command buffer to hold transfer commands
    VkCommandBuffer commandBuffer;

    // Command buffer details
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    // Allocate command buffer from pool
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    // Information to begin the command buffer record
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // We're only using the command buffer once, so set up for one time submit

    // Begin recording transfer commands
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

static void endAndSubmitCommandBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer) {
    // End commands
    vkEndCommandBuffer(commandBuffer);

    // Queue submission information
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Submit transfer command to transfer queue and wait until it finishes
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    // Free temporary command buffer back to pool
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

[[maybe_unused]] static void copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                        VkDeviceSize bufferSize) {

    VkCommandBuffer transferComandBuffer = beginCommandBuffer(device, transferCommandPool);

    // Region of data to copy from and to
    VkBufferCopy bufferCopyRegion = {};
    bufferCopyRegion.srcOffset = 0;
    bufferCopyRegion.dstOffset = 0;
    bufferCopyRegion.size = bufferSize;

    // Command to copy src buffer to dst buffer
    vkCmdCopyBuffer(transferComandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

    endAndSubmitCommandBuffer(device, transferCommandPool, transferQueue, transferComandBuffer);
}

[[maybe_unused]] static void copyImageBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, VkBuffer srcBuffer, VkImage image,
                                             uint32_t width, uint32_t height) {
    // Create Buffer
    VkCommandBuffer transferComandBuffer = beginCommandBuffer(device, transferCommandPool);

    VkBufferImageCopy imageRegion = {};
    imageRegion.bufferOffset = 0;                                             // Offset into data
    imageRegion.bufferRowLength = 0;                                          // Row leght of data to calculate data spacing
    imageRegion.bufferImageHeight = 0;                                        // Image height to calculate data spacing
    imageRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;      // Which aspect of image to copy
    imageRegion.imageSubresource.mipLevel = 0;                                // Mipmap level to copy
    imageRegion.imageSubresource.baseArrayLayer = 0;                          // Starting array layer (if array)
    imageRegion.imageSubresource.layerCount = 1;                              // Number of layers to copy starting ar baseArray
    imageRegion.imageOffset = {.x = 0, .y = 0, .z = 0};                       // Offset into image (as opposed to raw data in bufferOffset)
    imageRegion.imageExtent = {.width = width, .height = height, .depth = 1}; // Size of region to copy as (x, y, z) values

    // Copy buffer to given image
    vkCmdCopyBufferToImage(transferComandBuffer, srcBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageRegion);

    endAndSubmitCommandBuffer(device, transferCommandPool, transferQueue, transferComandBuffer);
}

[[maybe_unused]] static void transitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image, VkImageLayout oldLayout,
                                                   VkImageLayout newLayout) {
    // Create buffer
    VkCommandBuffer commandBuffer = beginCommandBuffer(device, commandPool);

    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.oldLayout = oldLayout;                                   // Layout to transition from
    imageMemoryBarrier.newLayout = newLayout;                                   // layout to transition to
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;           // Queue Falmily to transition from
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;           // Queue Family to transition to
    imageMemoryBarrier.image = image;                                           // Image being accessd and modified as part of barrier
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // aspect of image being altered
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;                       // First mip level to start alterations on
    imageMemoryBarrier.subresourceRange.levelCount = 1;                         // Number of mip levels to alter starting from maseMipLevel
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;                     // First layer to start aterarion on
    imageMemoryBarrier.subresourceRange.layerCount = 1;                         // Number of layers to alter starting from baseArrayLayer

    // VkPipelineStageFlags srcStage;
    // VkPipelineStageFlags dstStage;
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_NONE;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_NONE;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

        // if transitioning from new image to image ready to receive data..
        imageMemoryBarrier.srcAccessMask = 0;                            // Memory access stage transition must after ..
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Memory access stage transition must before ..

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

        // if transition from transfer destination to shade readable..
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(commandBuffer,         //
                         srcStage, dstStage,    // Pipelane stages (match to src and dst AccessMask)
                         0,                     // Dependency flags
                         0, nullptr,            // Memory Barrier cont + data
                         0, nullptr,            // Buffer Memory Barrier cont + data
                         1, &imageMemoryBarrier // Image Memory Barrier cont + data
    );

    endAndSubmitCommandBuffer(device, commandPool, queue, commandBuffer);
}
