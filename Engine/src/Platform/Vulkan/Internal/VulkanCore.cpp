#include "Platform/Vulkan/Internal/VulkanCore.h"

#include <vector>

#include <SDL3/SDL_vulkan.h>

#include "Utils/Log.h"
#include "Utils/Assert.h"
#include "Utils/Common.h"
#include "Platform/Vulkan/Internal/VulkanUtils.h"

namespace aero3d {

std::unique_ptr<VulkanCore> g_VulkanCore = std::make_unique<VulkanCore>();

VulkanCore::VulkanCore()
    : m_Window(nullptr), m_Instance(VK_NULL_HANDLE), m_Surface(VK_NULL_HANDLE),
    m_CurrentPhysDevice(0)
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

    A3D_CHECK_INIT(CreateInstance());
    A3D_CHECK_INIT(CreateSurface());
    A3D_CHECK_INIT(CreatePhysicalDevices());
    A3D_CHECK_INIT(m_Device.Init());

    return true;
}

void VulkanCore::Shutdown()
{
    m_Device.Shutdown();
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
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

bool VulkanCore::CreateSurface()
{
    if (!SDL_Vulkan_CreateSurface(m_Window, m_Instance, nullptr, &m_Surface)) {
        LogErr(ERROR_INFO, "Failed to create Vulkan surface: %s", SDL_GetError());
        return false;
    }

    return true;
}

bool VulkanCore::CreatePhysicalDevices()
{
    uint32_t physDeviceCount;
    A3D_CHECK_VKRESULT(vkEnumeratePhysicalDevices(m_Instance, &physDeviceCount, nullptr));

    if (!physDeviceCount)
    {
        LogErr(ERROR_INFO, "Failed to Enum PhysDevices.");
        return false;
    }

    std::vector<VkPhysicalDevice> vkDevices(physDeviceCount);
    m_PhysDevices.reserve(physDeviceCount);

    A3D_CHECK_VKRESULT(vkEnumeratePhysicalDevices(m_Instance, &physDeviceCount, vkDevices.data()));

    for (auto& vkDevice : vkDevices) {
        VulkanPhysicalDevice physDev{};
        physDev.Device = vkDevice;

        vkGetPhysicalDeviceProperties(vkDevice, &physDev.Properties);
        vkGetPhysicalDeviceFeatures(vkDevice, &physDev.Features);
        vkGetPhysicalDeviceMemoryProperties(vkDevice, &physDev.MemoryProperties);

        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vkDevice, &queueCount, nullptr);

        physDev.QueueFamilyProperties.resize(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkDevice, &queueCount, physDev.QueueFamilyProperties.data());

        for (uint32_t i = 0; i < physDev.QueueFamilyProperties.size(); i++)
        {
            if (physDev.QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
            {
                physDev.QueueFamilyIndices.GraphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(vkDevice, i, m_Surface, &presentSupport);

            if (presentSupport) 
            {
                physDev.QueueFamilyIndices.PresentFamily = i;
            }
        }

        if (physDev.QueueFamilyIndices.IsComplete())
        {
            m_PhysDevices.emplace_back(physDev);
        }
    }

    Assert(ERROR_INFO, !m_PhysDevices.empty(), "Unnable to Find Suitable Physical Device.");

    return true;
}

} // namespace aero3d