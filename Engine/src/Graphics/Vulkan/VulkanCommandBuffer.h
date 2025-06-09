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

    virtual void BindBuffer(Ref<Buffer> buffer) override;
    virtual void BindPipeline(Ref<Pipeline> pipeline) override;

    virtual void Execute() override;

    virtual Ref<Buffer> CreateBuffer(Buffer::Description desc) override;

    VkCommandBuffer GetCommandBuffer() { return m_GraphicsCB; }

private:
    void CreateCommandBuffers();
    void CreateQueue();

private:
    Ref<VulkanContext> m_Context = nullptr;
    Ref<VulkanViewport> m_Viewport = nullptr;

    VkCommandBuffer m_GraphicsCB = VK_NULL_HANDLE;
    VkCommandBuffer m_TransferCB = VK_NULL_HANDLE;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkFence m_Fence = VK_NULL_HANDLE;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANCOMMANDBUFFER_H_