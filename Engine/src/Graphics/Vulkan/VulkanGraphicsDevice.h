#ifndef AERO3D_GRAPHICS_VULKAN_VULKANGRAPHICSDEVICE_H_
#define AERO3D_GRAPHICS_VULKAN_VULKANGRAPHICSDEVICE_H_

#include <volk.h>

#include "Utils/Common.h"
#include "Graphics/Vulkan/VulkanBootstrap.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/Vulkan/VulkanResourceFactory.h"
#include "Graphics/Vulkan/VulkanSwapchain.h"

namespace aero3d {

class VulkanGraphicsDevice : public GraphicsDevice
{
public:
    VulkanGraphicsDevice(RenderSurfaceCreateInfo& renderSurfaceInfo);
    ~VulkanGraphicsDevice();

    virtual Ref<CommandList> CreateCommandList() override;
    virtual ResourceFactory* GetResourceFactory() override;
    virtual Swapchain* GetSwapchain() override;

    virtual void SubmitCommands(Ref<CommandList> commandList) override;
    virtual void Present() override;

    virtual void UpdateBuffer(Ref<DeviceBuffer> buffer, void* data, size_t size, size_t offset = 0) override;
    virtual void UpdateTexture(Ref<Texture> texture, void* data, size_t size) override;

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

public:
    RenderSurfaceCreateInfo surfaceInfo;

    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkPhysicalDevice physDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties physDeviceProperties;
    VkPhysicalDeviceFeatures physDeviceFeatures;
    VkPhysicalDeviceMemoryProperties physDeviceMemoryProperties;
    std::vector<VkQueueFamilyProperties> physDeviceQueueFamilyProperties;
    uint32_t graphicsQueueIndex = 0;
    uint32_t presentQueueIndex = 0;

    VkDevice device = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer transferCommandBuffer = VK_NULL_HANDLE;

    VkFence transferFinishedFence = VK_NULL_HANDLE;
    VkFence renderFinishedFence = VK_NULL_HANDLE;

    VulkanSwapchain* swapchain = nullptr;
    VulkanDescriptorAllocator* descriptorAllocator = nullptr;
    VulkanResourceFactory* resourceFactory = nullptr;

private:
    void CreateInstance();
    void CreateSurface();
    void CreatePhysDevice();
    void CreateDevice();
    void CreateQueues();
    void CreateCommandBuffers();
    void CreateLocks();

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANGRAPHICSDEVICE_H_
