#include "Platform/Vulkan/Internal/VulkanCore.h"

#include <vector>

#include <SDL3/SDL_vulkan.h>

#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Platform/Vulkan/Internal/VulkanUtils.h"

namespace aero3d {

std::unique_ptr<VulkanCore> g_VulkanCore = std::make_unique<VulkanCore>();

VulkanCore::VulkanCore()
    : m_Window(nullptr), m_Instance(VK_NULL_HANDLE), m_Surface(VK_NULL_HANDLE),
    m_PhysDeviceCount(0), m_CurrentPhysDevice(0)
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

    return true;
}

void VulkanCore::Shutdown()
{
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
    A3D_CHECK_VKRESULT(vkEnumeratePhysicalDevices(m_Instance, &m_PhysDeviceCount, nullptr));

    if (!m_PhysDeviceCount)
    {
        LogErr(ERROR_INFO, "Failed to Enum PhysDevices.");
        return false;
    }

    std::vector<VkPhysicalDevice> vkDevices(m_PhysDeviceCount);
    m_PhysDevices.reserve(m_PhysDeviceCount);

    A3D_CHECK_VKRESULT(vkEnumeratePhysicalDevices(m_Instance, &m_PhysDeviceCount, vkDevices.data()));

    for (auto& vkDevice : vkDevices) {
        VulkanPhysicalDevice physDev{};
        physDev.Device = vkDevice;

        vkGetPhysicalDeviceProperties(vkDevice, &physDev.Properties);
        vkGetPhysicalDeviceFeatures(vkDevice, &physDev.Features);
        vkGetPhysicalDeviceMemoryProperties(vkDevice, &physDev.MemoryProperties);

        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vkDevice, &queueCount, nullptr);

        physDev.QueueFamilyProperties.reserve(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkDevice, &queueCount, physDev.QueueFamilyProperties.data());

        m_PhysDevices.emplace_back(physDev);
    }

    return true;
}

} // namespace aero3d