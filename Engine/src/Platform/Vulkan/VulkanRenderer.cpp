#include "Platform/Vulkan/VulkanRenderer.h"

#include "Platform/Vulkan/Internal/VulkanContext.h"

namespace aero3d {

void VulkanRenderer::Init(SDL_Window* window, int width, int height)
{
    g_VulkanContext.Init(window, width, height);
}

void VulkanRenderer::Shutdown()
{
    g_VulkanContext.Shutdown();
}

void VulkanRenderer::BeginFrame()
{

}

void VulkanRenderer::EndFrame()
{

}

void VulkanRenderer::SubmitCommand(RenderCommand command)
{
    
}

void VulkanRenderer::ResizeBuffers()
{

}

} // namespace aero3d