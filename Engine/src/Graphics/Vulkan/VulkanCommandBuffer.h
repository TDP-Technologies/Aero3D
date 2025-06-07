#ifndef AERO3D_GRAPHICS_VULKAN_VULKANCOMMANDBUFFER_H_
#define AERO3D_GRAPHICS_VULKAN_VULKANCOMMANDBUFFER_H_

#include "Utils/Common.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanViewport.h"

namespace aero3d {

class VulkanCommandBuffer : public CommandBuffer
{
public:
    VulkanCommandBuffer(Ref<VulkanContext> context, Ref<VulkanViewport> viewport);
    ~VulkanCommandBuffer();

    virtual void Record() override;
    virtual void End() override;

    VkCommandBuffer GetCommandBuffer() { return m_CommandBuffer; }

private:
    void CreateCommandBuffer();

private:
    Ref<VulkanContext> m_Context = nullptr;
    Ref<VulkanViewport> m_Viewport = nullptr;

    VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANCOMMANDBUFFER_H_