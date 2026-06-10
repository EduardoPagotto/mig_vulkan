#include "Renderer.hpp"
#include <array>

namespace ce {

    Renderer::Renderer(std::shared_ptr<VWrapp> vwrapp, std::shared_ptr<ce::SwapChain> swc) : vwrapp(vwrapp), swc(swc) { // NOLINT
        //
        createRenderPass();
    }
    Renderer::~Renderer() {
        //

        vkDestroyRenderPass(vwrapp->getLogical(), this->renderPass, nullptr);
    }

    void Renderer::createRenderPass() {

        // ATTACHEMNTS
        // Colour attachment of render pass
        VkAttachmentDescription colourAttachemnt = {};
        colourAttachemnt.format = this->swc->getSwapchainImageFormat();     // Format to use for attachment
        colourAttachemnt.samples = VK_SAMPLE_COUNT_1_BIT;                   // Number of samples to write for multisampling
        colourAttachemnt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;              // Describes what to do with attachemnt before rendering
        colourAttachemnt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;            // Describes what todo with attachment after rendering
        colourAttachemnt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // Describes what todo with stencil before rendering
        colourAttachemnt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // Describes what todo with stencil before rendering

        // Framebuffer data will be storage as an image, but images can be given different data layouts
        // to give optimal use for certan operations
        colourAttachemnt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;     // Image data layout before render pass starts
        colourAttachemnt.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Imagedata layout after render pass (to change to)

        // depth attachemnt of render pass
        VkAttachmentDescription depthAttachemnt = {};
        depthAttachemnt.format = vwrapp->chooseSupportedFormat({VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT}, // Formats
                                                               VK_IMAGE_TILING_OPTIMAL,                                                           // Tilling
                                                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        depthAttachemnt.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachemnt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachemnt.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachemnt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachemnt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachemnt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachemnt.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // REFERENCES
        // Attachemnt reference uses an attachemnt index that refer to index in the attachemnt list passes to
        // renderPassCreateInfo
        VkAttachmentReference colourAttachmentReference = {};
        colourAttachmentReference.attachment = 0;
        colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Depth Attachment Refence
        VkAttachmentReference depthAttachemntReference = {};
        depthAttachemntReference.attachment = 1;
        depthAttachemntReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Information abaout a particular subpass the render pass is using
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Pipeline type subpass is to be bound to
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colourAttachmentReference;
        subpass.pDepthStencilAttachment = &depthAttachemntReference;

        // Need to determine when layout transitions occour subpass dependencies
        std::array<VkSubpassDependency, 2> subpassDependencies;

        // Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHEMNT_OPTIMAL
        // Transition must happen after..
        subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; // Subpass index (VK_SUBPASS_EXTERNAL = Special value means outside of renderpass)
        subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // Pipeline stage
        subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;           // Stage access mask (memory access)

        // But must happen before..
        subpassDependencies[0].dstSubpass = 0;
        subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependencies[0].dependencyFlags = 0;

        //
        // -----
        // Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHEMNT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        // Transition must happen after..
        subpassDependencies[1].srcSubpass = 0;
        subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        // But must happen before..
        subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDependencies[1].dependencyFlags = 0;

        std::array<VkAttachmentDescription, 2> renderPassAttachemnts = {colourAttachemnt, depthAttachemnt};

        // Create Info for render pass
        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachemnts.size());
        renderPassCreateInfo.pAttachments = renderPassAttachemnts.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
        renderPassCreateInfo.pDependencies = subpassDependencies.data();

        VkResult result = vkCreateRenderPass(vwrapp->getLogical(), &renderPassCreateInfo, nullptr, &this->renderPass);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!!!");
        }
    }
} // namespace ce
