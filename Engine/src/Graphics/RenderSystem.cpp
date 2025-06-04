#include "Graphics/RenderSystem.h"

#include "Utils/Log.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace aero3d {

Ref<Renderer> RenderSystem::s_Renderer = nullptr;

void RenderSystem::Init(SDL_Window* window, int height, int width)
{
    LogMsg("Render System Initialize.");

    s_Renderer = std::make_shared<VulkanRenderer>();

    s_Renderer->Init(window, height, width);
}

void RenderSystem::Shutdown()
{
    LogMsg("Render System Shutdown.");

    s_Renderer->Shutdown();
}

} // namespace aero3d