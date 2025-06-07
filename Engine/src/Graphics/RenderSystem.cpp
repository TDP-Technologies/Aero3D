#include "Graphics/RenderSystem.h"

#include "Utils/Log.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanViewport.h"

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

} // namespace aero3d