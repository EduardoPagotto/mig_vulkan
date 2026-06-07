#include "SwapChain.hpp"

namespace ce {

    SwapChain::SwapChain(std::shared_ptr<Device> dev) : dev(dev) { // NOLINT

        // Get Swap Chain details so we cam pick best setting
        SwapChainDetails swapchainDetails = this->dev->getSwapChainDetails(dev->getPhysicalDevice()); // FIXME: alterar assinatura do metodo

        // Find optimal surface value for our swap chain
        VkSurfaceFormatKHR surrfaceFormat = SwapChain::chooseBestSurfaceFormat(swapchainDetails.formats);

        VkPresentModeKHR presentMode = SwapChain::chooseBestPresentationMode(swapchainDetails.presentationModes);
        VkExtent2D extent = this->dev->chooseSwapExtent(swapchainDetails.surfaceCapabilities);

        // how many images are in the swap chain? Get 1 more than the minimum to allow triple buffering
        uint32_t imageCount = swapchainDetails.surfaceCapabilities.minImageCount + 1;

        // If imagecount higher than max the clamp down to max
        // If 0, then limitless
        if (swapchainDetails.surfaceCapabilities.maxImageCount > 0 && swapchainDetails.surfaceCapabilities.maxImageCount < imageCount) {
            imageCount = swapchainDetails.surfaceCapabilities.maxImageCount;
        }

        // Create information for swap chain
        VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = this->dev->getSurface();                                    // Swapchain surface
        swapchainCreateInfo.imageFormat = surrfaceFormat.format;                                  // Swapchain format
        swapchainCreateInfo.imageColorSpace = surrfaceFormat.colorSpace;                          // Swapchain color space
        swapchainCreateInfo.presentMode = presentMode;                                            // Swapchain presentation mode
        swapchainCreateInfo.imageExtent = extent;                                                 // Swapchain image extents
        swapchainCreateInfo.minImageCount = imageCount;                                           // Minimum image in swapchain
        swapchainCreateInfo.imageArrayLayers = 1;                                                 // Number of layers for each image in chain
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;                     // What attachement image will be used as
        swapchainCreateInfo.preTransform = swapchainDetails.surfaceCapabilities.currentTransform; // Transform to perform on swap chain images
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // How to handle blending images with external graphics(e.g. other windows)
        swapchainCreateInfo.clipped = VK_TRUE; // Whether to clip parts of image not in view (e.g. behind another window, off screen, etc)

        // Get Queue Family indices
        ce::QueueFamilyIndices indices = this->dev->getQueueFamilies(dev->getPhysicalDevice());

        // If Graphics and Presentation families are diferent, the swapchain must let images ge shared between families
        if (indices.graphicsFamily != indices.presentationFamily) {

            // Queue to share between
            uint32_t queueFamilyIndices[] = {(uint32_t)indices.graphicsFamily, (uint32_t)indices.presentationFamily};

            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Image share handling
            swapchainCreateInfo.queueFamilyIndexCount = 2;                     // Number of queues to share images between
            swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;      // Array of queues to share between
        } else {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchainCreateInfo.queueFamilyIndexCount = 0;
            swapchainCreateInfo.pQueueFamilyIndices = nullptr;
        }

        // If old swap chain been destroyed and this one replaces it, then link old one to quickly hand over
        // responsabilities
        swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        // Create Swapchain
        VkResult result = vkCreateSwapchainKHR(dev->getLogical(), &swapchainCreateInfo, nullptr, &this->swapchain);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create a Swapchain");
        }

        // Store for late reference
        this->swapchainImageFormat = surrfaceFormat.format;
        this->swapchainExtent = extent;

        // Get swap chain images (first count the values)
        uint32_t swapChainImageCount;
        vkGetSwapchainImagesKHR(dev->getLogical(), this->swapchain, &swapChainImageCount, nullptr);

        std::vector<VkImage> images(swapChainImageCount);
        vkGetSwapchainImagesKHR(dev->getLogical(), this->swapchain, &swapChainImageCount, images.data());

        for (VkImage image : images) {
            // Store image handle
            SwapchainImage swapChainImage = {};
            swapChainImage.image = image;
            swapChainImage.imageView = this->createImageView(image, this->swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

            // Add to swapchain image list
            this->swapchainImages.push_back(swapChainImage);
        }

    } // NOLINT

    SwapChain::~SwapChain() {

        for (auto image : this->swapchainImages) {
            vkDestroyImageView(dev->getLogical(), image.imageView, nullptr);
        }

        vkDestroySwapchainKHR(dev->getLogical(), this->swapchain, nullptr);
    }

    VkImageView SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const {
        //
        VkImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.image = image;                                // Image to create view for
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;             // Type of image (1D, 2D, 3D, Cube, etc)
        viewCreateInfo.format = format;                              // Format of image data
        viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // Allows remapping of rgba component to other values
        viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // Subresources allow the view to view only a part of a image
        viewCreateInfo.subresourceRange.aspectMask = aspectFlags; // which aspect of image to view (e.g. COLOR_BIT for view color)
        viewCreateInfo.subresourceRange.baseMipLevel = 0;         // Start mipmap level to start from
        viewCreateInfo.subresourceRange.levelCount = 1;           // Number of mipmap levels to view
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;       // Start array level to view from
        viewCreateInfo.subresourceRange.layerCount = 1;           // Numbers of array levels to view

        // Create image view and return it
        VkImageView imageView;
        VkResult result = vkCreateImageView(dev->getLogical(), &viewCreateInfo, nullptr, &imageView);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create an Image View!");
        }

        return imageView;
    }

    // Best format is subjective, but ours will be:
    // Format     : VK_FORMAT_R8G8B8A8_UNFORM (VK_FORMAT_B8G8R8A8_UNORM as backup)
    // colorSpace : VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    VkSurfaceFormatKHR SwapChain::chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {

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

    VkPresentModeKHR SwapChain::chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes) {
        // Look for Mailbox presentation mode
        for (const auto& presentationMode : presentationModes) {
            if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return presentationMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR; // allways avaible by vulkan
    }

} // namespace ce
