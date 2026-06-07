#pragma once
#include "SwapChain.hpp"

namespace ce {

    class Renderer {
      public:
        explicit Renderer(std::shared_ptr<Device> dev, std::shared_ptr<ce::SwapChain> swc);
        virtual ~Renderer();

        VkRenderPass& getRenderPass() { return renderPass; }

      private:
        std::shared_ptr<ce::Device> dev;
        std::shared_ptr<ce::SwapChain> swc;
        VkRenderPass renderPass;

        void createRenderPass();
    };
} // namespace ce
