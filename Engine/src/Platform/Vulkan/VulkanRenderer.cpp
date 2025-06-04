#include "Platform/Vulkan/VulkanRenderer.h"

#include "Platform/Vulkan/Internal/VulkanContext.h"

namespace aero3d {

void VulkanRenderer::Init(SDL_Window* window, int height, int width)
{
    g_VulkanContext.Init(window);
}

void VulkanRenderer::Shutdown()
{
    g_VulkanContext.Shutdown();
}

void VulkanRenderer::SubmitCommand(RenderCommand command)
{
    
}

void VulkanRenderer::ResizeBuffers()
{

}

} // namespace aero3d