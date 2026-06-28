#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstddef>
#include <fstream>
#include <glm/glm.hpp>
#include <ios>
#include <stdexcept>
#include <vector>

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
