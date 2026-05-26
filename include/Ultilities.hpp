#pragma once
#include <cstddef>
#include <fstream>
#include <glm/glm.hpp>
#include <ios>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

const int MAX_FRAME_DRAWS = 2;

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
