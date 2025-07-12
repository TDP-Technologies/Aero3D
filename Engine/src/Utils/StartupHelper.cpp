#include "Utils/StartupHelper.h"

#include "Graphics/Vulkan/VulkanGraphicsDevice.h"

namespace aero3d {

GraphicsDevice* StartupHelper::CreateGraphicsDevice(RenderingAPI api, RenderSurfaceCreateInfo surfaceInfo)
{
    switch (api)
    {
    case RenderingAPI::Vulkan: return new VulkanGraphicsDevice(surfaceInfo);
    default: return nullptr;
    }
}

Window* StartupHelper::CreateWindow(WindowInfo info)
{
    return new Window(info);
}

} // namespace aero3d
