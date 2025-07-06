#ifndef AERO3D_GRAPHICS_VULKAN_VULKANCOMMANDLIST_H_
#define AERO3D_GRAPHICS_VULKAN_VULKANCOMMANDLIST_H_

#include <cstdint>

#include <volk.h>

#include "Graphics/CommandList.h"
#include "Graphics/Vulkan/VulkanResources.h"

namespace aero3d {

class VulkanGraphicsDevice;

class VulkanCommandList : public CommandList
{
public:
    VulkanCommandList(VulkanGraphicsDevice* gd);
    ~VulkanCommandList();

    virtual void Begin() override;
    virtual void End() override;

    virtual void SetFramebuffer(Ref<Framebuffer> framebuffer) override;
    virtual void SetPipeline(Ref<Pipeline> pipeline) override;
    virtual void SetVertexBuffer(Ref<DeviceBuffer> buffer, uint32_t offset = 0) override;
    virtual void SetIndexBuffer(Ref<DeviceBuffer> buffer, IndexFormat format, uint32_t offset = 0) override;
    virtual void SetResourceSet(uint32_t slot, Ref<ResourceSet> resourceSet) override;

    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1) override;
    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1) override;

public:
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

private:
    void CreateCommandPool();
    void CreateCommandList();

    void BeginRendering();
    void EndRendering();

private:
    VulkanGraphicsDevice* m_GraphicsDevice = nullptr;

    VkCommandPool m_CommandPool = VK_NULL_HANDLE;

    uint32_t m_CurrentImage = 0;
    Ref<VulkanFramebuffer> m_CurrentFramebuffer = nullptr;
    Ref<VulkanPipeline> m_CurrentPipeline = nullptr;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANCOMMANDLIST_H_