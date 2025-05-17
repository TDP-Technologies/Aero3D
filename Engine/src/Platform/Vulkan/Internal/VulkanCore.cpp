#include "Platform/Vulkan/Internal/VulkanCore.h"

#include <vector>
#include <algorithm>

#include <SDL3/SDL_vulkan.h>

#include "Utils/Log.h"
#include "Utils/Assert.h"
#include "Utils/Common.h"
#include "Platform/Vulkan/Internal/VulkanUtils.h"

namespace aero3d {

std::unique_ptr<VulkanCore> g_VulkanCore = std::make_unique<VulkanCore>();

static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) 
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) 
    {
        details.Formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) 
    {
        details.PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
    }

    return details;
}

static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
{
    for (const auto& format : availableFormats) 
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
        {
            return format;
        }
    }
    return availableFormats[0];
}

static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
{
    for (const auto& mode : availablePresentModes) 
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) 
        {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, SDL_Window* window) 
{
    if (capabilities.currentExtent.width != UINT32_MAX) 
    {
        return capabilities.currentExtent;
    }
    else 
    {
        int width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width);

        actualExtent.height = std::clamp(actualExtent.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VulkanCore::VulkanCore()
    : m_Window(nullptr), m_Instance(VK_NULL_HANDLE), m_Surface(VK_NULL_HANDLE),
    m_Swapchain(VK_NULL_HANDLE), m_CurrentPhysDevice(0)
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
    A3D_CHECK_INIT(CreateSwapchain());

    return true;
}

void VulkanCore::Shutdown()
{
    vkDestroySwapchainKHR(m_Device.GetDevice(), m_Swapchain, nullptr);
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

bool VulkanCore::CreateSwapchain()
{
    VulkanPhysicalDevice& physDevice = m_PhysDevices[m_CurrentPhysDevice];
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physDevice.Device, m_Surface);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities, m_Window);

    uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
    if (swapChainSupport.Capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.Capabilities.maxImageCount) 
    {
        imageCount = swapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = physDevice.QueueFamilyIndices;
    uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

    if (indices.GraphicsFamily != indices.PresentFamily) 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    A3D_CHECK_VKRESULT(vkCreateSwapchainKHR(m_Device.GetDevice(), &createInfo, nullptr, &m_Swapchain));
    if (!m_Swapchain)
    {
        LogErr(ERROR_INFO, "Failed to create Swapchain.");
        return false;
    }

    return true;
}

} // namespace aero3d