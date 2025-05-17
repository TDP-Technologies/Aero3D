#include "Platform/Vulkan/Internal/VulkanCore.h"

#include <vector>

#include <SDL3/SDL_vulkan.h>

#include "Utils/Log.h"

#include "Platform/Vulkan/Internal/VulkanUtils.h"

namespace aero3d {

VulkanCore* g_VulkanCore = new VulkanCore();

VulkanCore::VulkanCore()
    : m_Window(nullptr), m_Instance(VK_NULL_HANDLE)
{
}

VulkanCore::~VulkanCore()
{
}

bool VulkanCore::Init(SDL_Window* window)
{
    m_Window = window;

    if (!SDL_Vulkan_LoadLibrary(nullptr)) {
        LogErr(ERROR_INFO, "Failed to load Vulkan library: %s", SDL_GetError());
        return false;
    }

    if (!CreateInstance())
    {
        return false;
    }

    return true;
}

void VulkanCore::Shutdown()
{
    vkDestroyInstance(m_Instance, nullptr);
}

bool VulkanCore::CreateInstance()
{
    uint32_t extensionCount;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    if (!extensions) {
        LogErr(ERROR_INFO, "Could not get Vulkan instance extensions: %s", SDL_GetError());
        return false;
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Aero3D";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledLayerCount = 0;

    A3D_CHECK_VKRESULT(vkCreateInstance(&createInfo, nullptr, &m_Instance));

    return true;
}

} // namespace aero3d