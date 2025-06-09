#include "Graphics/RenderSystem.h"

#include "Utils/Log.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanViewport.h"
#include "Graphics/Vulkan/VulkanCommandBuffer.h"
#include "Graphics/Vulkan/VulkanResources.h"

namespace aero3d {

void RenderSystem::Init()
{
    LogMsg("Render System Initialize.");

}

void RenderSystem::Shutdown()
{
    LogMsg("Render System Shutdown.");

}

Ref<Context> RenderSystem::CreateContext(SDL_Window* window)
{
    return std::make_shared<VulkanContext>(window);
}

Ref<Viewport> RenderSystem::CreateViewport(Ref<Context> context, int width, int height)
{
    return std::make_shared<VulkanViewport>(std::static_pointer_cast<VulkanContext>(context), width, height);
}

Ref<CommandBuffer> RenderSystem::CreateCommandBuffer(Ref<Context> context, Ref<Viewport> viewport)
{
    return std::make_shared<VulkanCommandBuffer>(std::static_pointer_cast<VulkanContext>(context), 
        std::static_pointer_cast<VulkanViewport>(viewport));
}

Ref<Pipeline> RenderSystem::CreatePipeline(Ref<Context> context, Ref<Viewport> viewport, Pipeline::Description desc)
{
    return std::make_shared<VulkanPipeline>(std::static_pointer_cast<VulkanContext>(context),
        std::static_pointer_cast<VulkanViewport>(viewport), desc);
}

} // namespace aero3d