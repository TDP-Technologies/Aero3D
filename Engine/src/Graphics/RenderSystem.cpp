#include "Graphics/RenderSystem.h"

#include "Utils/Log.h"

namespace aero3d {

void RenderSystem::Init(SDL_Window* window, int height, int width)
{
    LogMsg("Render System Initialize.");

}

void RenderSystem::Shutdown()
{
    LogMsg("Render System Shutdown.");

}

} // namespace aero3d