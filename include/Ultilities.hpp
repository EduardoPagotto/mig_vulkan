#pragma once

#define GLFW_INCLUDE_VULKAN
#include "Command.hpp"
#include <GLFW/glfw3.h>
#include <cstddef>
#include <fstream>
#include <glm/glm.hpp>
#include <ios>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

const int MAX_FRAME_DRAWS = 2;
const int MAX_OBJECTS = 30;

// Vertex data representation
struct Vertex {
    glm::vec3 pos; // Vertex Position (x, y, z)
    glm::vec3 col; // Vertex Color (r, g, b)
    glm::vec2 tex; // Texture Coords (u, v)
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

static void submitQueue(VkQueue queue, VkCommandBuffer commandBuffer) {
    // Queue submission information
    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, //
        .commandBufferCount = 1,                //
        .pCommandBuffers = &commandBuffer       //
    };

    // Submit transfer command to transfer queue and wait until it finishes
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
}

[[maybe_unused]] static void copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, VkBuffer srcBuffer,
                                        VkBuffer dstBuffer, VkDeviceSize bufferSize) {

    ce::CommandBuffer transferComandBuffer(device, transferCommandPool, 1);
    transferComandBuffer.begin(0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    // Region of data to copy from and to
    const VkBufferCopy bufferCopyRegion{.srcOffset = 0, .dstOffset = 0, .size = bufferSize};

    // Command to copy src buffer to dst buffer
    vkCmdCopyBuffer(transferComandBuffer.getBuffers()[0], srcBuffer, dstBuffer, 1, &bufferCopyRegion);

    transferComandBuffer.end(0);
    submitQueue(transferQueue, transferComandBuffer.getBuffers()[0]);
}

[[maybe_unused]] static void copyImageBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, VkBuffer srcBuffer,
                                             VkImage image, uint32_t width, uint32_t height) {
    // Create Buffer
    ce::CommandBuffer transferComandBuffer(device, transferCommandPool, 1);
    transferComandBuffer.begin(0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    const VkBufferImageCopy imageRegion{
        .bufferOffset = 0,                                                                    // Offset into data
        .bufferRowLength = 0,                                                                 // Row leght of data to calculate data spacing
        .bufferImageHeight = 0,                                                               // Image height to calculate data spacing
        .imageSubresource = VkImageSubresourceLayers{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, // Which aspect of image to copy
                                                     .mipLevel = 0,                           // Mipmap level to copy
                                                     .baseArrayLayer = 0,                     // Starting array layer (if array)
                                                     .layerCount = 1}, // Number of layers to copy starting ar baseArray
        .imageOffset = VkOffset3D{.x = 0, .y = 0, .z = 0},             // Offset into image (as opposed to raw data in bufferOffset)
        .imageExtent = VkExtent3D{.width = width, .height = height, .depth = 1} // Size of region to copy as (x, y, z) values
    };

    // Copy buffer to given image
    vkCmdCopyBufferToImage(transferComandBuffer.getBuffers()[0], srcBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageRegion);

    transferComandBuffer.end(0);
    submitQueue(transferQueue, transferComandBuffer.getBuffers()[0]);
}

[[maybe_unused]] static void transitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image,
                                                   VkImageLayout oldLayout, VkImageLayout newLayout) {
    // Create buffer
    ce::CommandBuffer commandBuffer(device, commandPool, 1);
    commandBuffer.begin(0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_NONE;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_NONE;

    // if transitioning from new image to image ready to receive data..
    VkAccessFlags srcAccessMask = 0;                            // Memory access stage transition must after ..
    VkAccessFlags dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Memory access stage transition must before ..

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

        // if transition from transfer destination to shade readable..
        srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    const VkImageMemoryBarrier imageMemoryBarrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                                  .srcAccessMask = srcAccessMask,
                                                  .dstAccessMask = dstAccessMask,
                                                  .oldLayout = oldLayout,                         // Layout to transition from
                                                  .newLayout = newLayout,                         // layout to transition to
                                                  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, // Queue Falmily to transition from
                                                  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, // Queue Family to transition to
                                                  .image = image, // Image being accessd and modified as part of barrier
                                                  .subresourceRange = VkImageSubresourceRange{
                                                      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, // aspect of image being altered
                                                      .baseMipLevel = 0,                       // First mip level to start alterations on
                                                      .levelCount = 1,     // Number of mip levels to alter starting from maseMipLevel
                                                      .baseArrayLayer = 0, // First layer to start aterarion on
                                                      .layerCount = 1      // Number of layers to alter starting from baseArrayLayer
                                                  }};

    vkCmdPipelineBarrier(commandBuffer.getBuffers()[0], //
                         srcStage, dstStage,            // Pipelane stages (match to src and dst AccessMask)
                         0,                             // Dependency flags
                         0, nullptr,                    // Memory Barrier cont + data
                         0, nullptr,                    // Buffer Memory Barrier cont + data
                         1, &imageMemoryBarrier         // Image Memory Barrier cont + data
    );

    commandBuffer.end(0);
    submitQueue(queue, commandBuffer.getBuffers()[0]);
}
