#ifndef AERO3D_GRAPHICS_VULKAN_VULKANQUEUE_H_
#define AERO3D_GRAPHICS_VULKAN_VULKANQUEUE_H_

#include "Utils/Common.h"
#include "Graphics/Queue.h"
#include "Graphics/Vulkan/VulkanContext.h"

namespace aero3d {

class VulkanQueue : public Queue
{
public:
    VulkanQueue(Ref<VulkanContext> context);
    ~VulkanQueue();

    virtual void Execute(Ref<CommandBuffer> buffer) override;

private:
    void CreateQueue();

private:
    Ref<VulkanContext> m_Context = nullptr;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkFence m_Fence = VK_NULL_HANDLE;

};
    
} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANQUEUE_H_