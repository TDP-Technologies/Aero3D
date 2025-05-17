#include "Platform/Vulkan/VulkanContext.h"

#include "Utils/Log.h"
#include "Platform/Vulkan/Internal/VulkanCore.h"

namespace aero3d {

VulkanContext::VulkanContext()
{
}

VulkanContext::~VulkanContext()
{
}

bool VulkanContext::Init(SDL_Window* window)
{
    LogMsg("Graphics Context Initialize.");

    if (!g_VulkanCore->Init(window))
    {
        return false;
    }

    return true;
}

void VulkanContext::Shutdown()
{
    LogMsg("Graphics Context Shutdown.");

    if (g_VulkanCore)
    {
        g_VulkanCore->Shutdown();
    }
}

void VulkanContext::SwapBuffers()
{

}

} // namespace aero3d