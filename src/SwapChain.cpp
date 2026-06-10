#include "SwapChain.hpp"
#include "VWrappUtils.hpp"

namespace ce {

    SwapChain::SwapChain(std::shared_ptr<VWrapp> vwrapp) : vwrapp(vwrapp) { // NOLINT

        // Get Swap Chain details so we cam pick best setting
        SwapChainDetails swapchainDetails =
            GetSwapChainDetails(vwrapp->getPhysical(), vwrapp->getSurface()); // FIXME: alterar assinatura do metodo

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

        // Create information for swap chain
        VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = vwrapp->getSurface();                   // Swapchain surface
        swapchainCreateInfo.imageFormat = surrfaceFormat.format;              // Swapchain format
        swapchainCreateInfo.imageColorSpace = surrfaceFormat.colorSpace;      // Swapchain color space
        swapchainCreateInfo.presentMode = presentMode;                        // Swapchain presentation mode
        swapchainCreateInfo.imageExtent = extent;                             // Swapchain image extents
        swapchainCreateInfo.minImageCount = imageCount;                       // Minimum image in swapchain
        swapchainCreateInfo.imageArrayLayers = 1;                             // Number of layers for each image in chain
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // What attachement image will be used as
        swapchainCreateInfo.preTransform =
            swapchainDetails.surfaceCapabilities.currentTransform; // Transform to perform on swap chain images
        swapchainCreateInfo.compositeAlpha =
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // How to handle blending images with external graphics(e.g. other windows)
        swapchainCreateInfo.clipped = VK_TRUE; // Whether to clip parts of image not in view (e.g. behind another window, off screen, etc)

        // Get Queue Family indices
        ce::QueueFamilyIndices indices = GetQueueFamilies(vwrapp->getPhysical(), vwrapp->getSurface());

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
        VkResult result = vkCreateSwapchainKHR(vwrapp->getLogical(), &swapchainCreateInfo, nullptr, &this->swapchain);
        if (result != VK_SUCCESS) {
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
            // Store image handle
            SwapchainImage swapChainImage = {};
            swapChainImage.image = image;
            swapChainImage.imageView = CreateImageView(vwrapp->getLogical(), image, this->swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

            // Add to swapchain image list
            this->swapchainImages.push_back(swapChainImage);
        }
    }

    SwapChain::~SwapChain() {

        for (auto image : this->swapchainImages) {
            vkDestroyImageView(vwrapp->getLogical(), image.imageView, nullptr);
        }

        vkDestroySwapchainKHR(vwrapp->getLogical(), this->swapchain, nullptr);
    }
} // namespace ce
