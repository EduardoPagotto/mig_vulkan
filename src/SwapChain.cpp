#include "SwapChain.hpp"
#include "ImageObject.hpp"
#include "VWrappUtils.hpp"
#include <array>
#include <memory>

namespace ce {

    SwapChain::SwapChain(std::shared_ptr<VWrapp> vwrapp) : vwrapp(vwrapp) { // NOLINT

        // Get Swap Chain details so we cam pick best setting
        SwapChainDetails swapchainDetails = GetSwapChainDetails(vwrapp->getPhysical(), vwrapp->getSurface());

        // Find optimal surface value for our swap chain
        VkSurfaceFormatKHR surrfaceFormat = ChooseBestSurfaceFormat(swapchainDetails.formats);

        VkPresentModeKHR presentMode = ChooseBestPresentationMode(swapchainDetails.presentationModes);
        VkExtent2D extent = vwrapp->chooseSwapExtent(swapchainDetails.surfaceCapabilities);

        // how many images are in the swap chain? Get 1 more than the minimum to allow triple buffering
        uint32_t imageCount = swapchainDetails.surfaceCapabilities.minImageCount + 1;

        // If imagecount higher than max the clamp down to max
        // If 0, then limitless
        if (swapchainDetails.surfaceCapabilities.maxImageCount > 0 && swapchainDetails.surfaceCapabilities.maxImageCount < imageCount) {
            imageCount = swapchainDetails.surfaceCapabilities.maxImageCount;
        }

        // Get Queue Family indices
        ce::QueueFamilyIndices indices = GetQueueFamilies(vwrapp->getPhysical(), vwrapp->getSurface());
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
        VkSwapchainCreateInfoKHR swapchainCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = vwrapp->getSurface(),                                       // Swapchain surface
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
        if (vkCreateSwapchainKHR(vwrapp->getLogical(), &swapchainCreateInfo, nullptr, &this->swapchain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create a Swapchain");
        }

        // Store for late reference
        this->swapchainImageFormat = surrfaceFormat.format;
        this->swapchainExtent = extent;

        // Get swap chain images (first count the values)
        uint32_t swapChainImageCount;
        vkGetSwapchainImagesKHR(vwrapp->getLogical(), this->swapchain, &swapChainImageCount, nullptr);

        std::vector<VkImage> images(swapChainImageCount);
        vkGetSwapchainImagesKHR(vwrapp->getLogical(), this->swapchain, &swapChainImageCount, images.data());

        for (VkImage image : images) {

            auto imgObj = std::make_shared<ImageObject>(this->vwrapp->getPhysical(), this->vwrapp->getLogical());
            imgObj->createImageViewImportedImage(image, this->swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT); // CreateImageView
            this->swapchainImages.push_back(imgObj);
        }
    }

    SwapChain::~SwapChain() {

        for (auto& image : this->swapchainImages) {
            image.reset();
        }

        vkDestroySwapchainKHR(vwrapp->getLogical(), this->swapchain, nullptr);
    }
} // namespace ce
