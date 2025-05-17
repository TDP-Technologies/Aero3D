#include "Platform/Vulkan/VulkanContext.h"

#include "Utils/Log.h"
#include "Utils/Common.h"
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

    A3D_CHECK_INIT(g_VulkanCore->Init(window));

    return true;
}

void VulkanContext::Shutdown()
{
    LogMsg("Graphics Context Shutdown.");

    A3D_SHUTDOWN(g_VulkanCore);
}

void VulkanContext::SwapBuffers()
{

}

} // namespace aero3d