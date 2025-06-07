#include "Graphics/RenderSystem.h"

#include "Utils/Log.h"
#include "Graphics/Vulkan/VulkanContext.h"

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

} // namespace aero3d