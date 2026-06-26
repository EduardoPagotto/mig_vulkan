#include "SwapChain.hpp"
#include "ImageObject.hpp"
#include "VWrappUtils.hpp"
#include <array>
#include <memory>

namespace ce {

#ifdef SET_GLFW_ENABLE
    SwapChain::SwapChain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, GLFWwindow* window)
#else
    SwapChain::SwapChain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, SDL_Window* window)
#endif
        : logicalDevice(logicalDevice), window(window) { // NOLINT

        // Get Swap Chain details so we cam pick best setting
        SwapChainDetails swapchainDetails = GetSwapChainDetails(physicalDevice, surface);

        // Find optimal surface value for our swap chain
        VkSurfaceFormatKHR surrfaceFormat = SwapChain::ChooseBestSurfaceFormat(swapchainDetails.formats);

        VkPresentModeKHR presentMode = SwapChain::ChooseBestPresentationMode(swapchainDetails.presentationModes);
        VkExtent2D extent = this->chooseSwapExtent(swapchainDetails.surfaceCapabilities);

        // how many images are in the swap chain? Get 1 more than the minimum to allow triple buffering
        uint32_t imageCount = swapchainDetails.surfaceCapabilities.minImageCount + 1;

        // If imagecount higher than max the clamp down to max
        // If 0, then limitless
        if (swapchainDetails.surfaceCapabilities.maxImageCount > 0 && swapchainDetails.surfaceCapabilities.maxImageCount < imageCount) {
            imageCount = swapchainDetails.surfaceCapabilities.maxImageCount;
        }

        // Get Queue Family indices
        ce::QueueFamilyIndices indices = GetQueueFamilies(physicalDevice, surface);
        // If Graphics and Presentation families are diferent, the swapchain must let images ge shared between families

        // indices.graphicsFamily == indices.presentationFamily
        VkSharingMode imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        uint32_t queueFamilyIndexCount = 0;
        const uint32_t* pQueueFamilyIndices = nullptr;

        // If Graphics and Presentation families are diferent, the swapchain must let images ge shared between families
        if (indices.graphicsFamily != indices.presentationFamily) {
            // Queue to share between
            std::array<uint32_t, 2> queueFamilyIndices = {static_cast<uint32_t>(indices.graphicsFamily),
                                                          static_cast<uint32_t>(indices.presentationFamily)};

            imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
            pQueueFamilyIndices = queueFamilyIndices.data();
        }

        // Create information for swap chain
        const VkSwapchainCreateInfoKHR swapchainCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,                                                    // Swapchain surface
            .minImageCount = imageCount,                                           // Minimum image in swapchain
            .imageFormat = surrfaceFormat.format,                                  // Swapchain format
            .imageColorSpace = surrfaceFormat.colorSpace,                          // Swapchain color space
            .imageExtent = extent,                                                 // Swapchain image extents
            .imageArrayLayers = 1,                                                 // Number of layers for each image in chain
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,                     // What attachement image will be used as
            .imageSharingMode = imageSharingMode,                                  // Image share handling
            .queueFamilyIndexCount = queueFamilyIndexCount,                        // Number of queues to share images between
            .pQueueFamilyIndices = pQueueFamilyIndices,                            // Array of queues to share between
            .preTransform = swapchainDetails.surfaceCapabilities.currentTransform, // Transform to perform on swap chain images
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // How to handle blending images with external graphics(e.g. other windows)
            .presentMode = presentMode,                          // Swapchain presentation mode
            .clipped = VK_TRUE,              // Whether to clip parts of image not in view (e.g. behind another window, off screen, etc)
            .oldSwapchain = VK_NULL_HANDLE}; //  If old swap chain been destroyed and this one replaces it, then link old one to quickly
                                             //  hand over  responsabilities

        // Create Swapchain
        if (vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, nullptr, &this->swapchain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create a Swapchain");
        }

        // Store for late reference
        this->imageFormat = surrfaceFormat.format;
        this->extent = extent;

        // Get swap chain images (first count the values)
        uint32_t swapChainImageCount;
        vkGetSwapchainImagesKHR(logicalDevice, this->swapchain, &swapChainImageCount, nullptr);

        std::vector<VkImage> lImages(swapChainImageCount);
        vkGetSwapchainImagesKHR(logicalDevice, this->swapchain, &swapChainImageCount, lImages.data());

        for (VkImage image : lImages) {

            auto imgObj = std::make_shared<ImageObject>(physicalDevice, logicalDevice);
            imgObj->createImageViewImportedImage(image, this->imageFormat, VK_IMAGE_ASPECT_COLOR_BIT); // CreateImageView
            this->images.push_back(imgObj);
        }
    }

    SwapChain::~SwapChain() {

        for (auto& framebuffer : this->swapChainFrameBuffers) { // ? auto& mesmo ??
            vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
        }

        for (auto& image : this->images) {
            image.reset();
        }

        vkDestroySwapchainKHR(logicalDevice, this->swapchain, nullptr);
    }

    VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {

        // If current extend!!!!!!!!!!!!
        if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return surfaceCapabilities.currentExtent;
        }

        int witdh;
        int height;
#ifdef SET_GLFW_ENABLE
        glfwGetFramebufferSize(this->window, &witdh, &height);
#else
        SDL_GetWindowSizeInPixels(this->window, &witdh, &height);
#endif
        VkExtent2D newExtent{
            .width = static_cast<uint32_t>(witdh),  //
            .height = static_cast<uint32_t>(height) //
        };

        // surface also defie max and min, so make sure within bondaries by clamping value
        newExtent.width =
            std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));

        newExtent.height =
            std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

        return newExtent;
    }

    void SwapChain::createFramebuffers(VkImageView& imageView, VkRenderPass& renderPass) {
        // Resize framebuffer count to equal chain image count
        this->swapChainFrameBuffers.resize(this->getImages().size());

        // Create a framebuffer for eache swap chain image
        for (size_t i = 0; i < this->swapChainFrameBuffers.size(); i++) {

            std::array<VkImageView, 2> attachments = {this->getImages()[i]->getImageView(), imageView}; // order important same as upper

            const VkFramebufferCreateInfo framebufferCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = renderPass,                                     // Render Pass layout the framebuffer will be used with
                .attachmentCount = static_cast<uint32_t>(attachments.size()), //
                .pAttachments = attachments.data(),                           // List of attachments (1:1 with Render Pass)
                .width = this->getExtent().width,                             // Framebuffer width
                .height = this->getExtent().height,                           // Framebuffer height
                .layers = 1                                                   // Framebuffer layers
            };

            if (vkCreateFramebuffer(logicalDevice, &framebufferCreateInfo, nullptr, &this->swapChainFrameBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Faleid to create a frambuffer");
            }
        }
    }

    // Best format is subjective, but ours will be:
    // Format     : VK_FORMAT_R8G8B8A8_UNFORM (VK_FORMAT_B8G8R8A8_UNORM as backup)
    // colorSpace : VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    VkSurfaceFormatKHR SwapChain::ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {

        // If only 1 format avaible and is undefined, them this means ALL formats ase avaible (no restricion)
        if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            return {.format = VK_FORMAT_R8G8B8A8_UNORM, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        }

        // If restriced, searche for optimal format
        for (const auto& format : formats) {
            if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }

        // If can't find optimal format, then just return first format
        return formats[0]; // FIXME: pade data pau aqui
    }

    VkPresentModeKHR SwapChain::ChooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes) {
        // Look for Mailbox presentation mode
        for (const auto& presentationMode : presentationModes) {
            if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return presentationMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR; // allways avaible by vulkan
    }

} // namespace ce
