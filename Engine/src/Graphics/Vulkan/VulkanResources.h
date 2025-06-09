#ifndef AERO3D_GRAPHICS_VULKAN_VULKANRESOURCES_H_
#define AERO3D_GRAPHICS_VULKAN_VULKANRESOURCES_H_

#include <volk.h>
#include <shaderc/shaderc.hpp>

#include "Utils/Common.h"
#include "Graphics/Resources.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanViewport.h"

namespace aero3d {

class VulkanPipeline : public Pipeline
{
public:
    VulkanPipeline(Ref<VulkanContext> context, Ref<VulkanViewport> viewport, Pipeline::Description desc);
    ~VulkanPipeline();

    VkPipeline GetPipeline() { return m_Pipeline; }

private:
    std::vector<uint32_t> CompileGLSL(const std::string& source, shaderc_shader_kind kind, const std::string& name);

    VkShaderModule CreateShaderModule(Pipeline::Shader& shader);
    void CreatePipelineLayout();
    void CreatePipeline(Ref<VulkanViewport> viewport, Pipeline::Description& vertexLayout);

private:
    Ref<VulkanContext> m_Context = nullptr;
    VkPipelineLayout m_Layout = VK_NULL_HANDLE;
    VkPipeline m_Pipeline = VK_NULL_HANDLE;

};

class VulkanBuffer : public Buffer
{
public:
    VulkanBuffer(Ref<VulkanContext> context, Buffer::Description desc);
    ~VulkanBuffer();

    Buffer::BufferType GetUsage() { return m_Usage; }
    VkIndexType GetIndexType() { return m_IndexType; }
    VkBuffer GetBuffer() { return m_Buffer; }
    VkDeviceMemory GetMemory() { return m_Memory; }
    uint32_t GetElementsCount() { return m_ElementsCount; }

private:
    Ref<VulkanContext> m_Context;
    Buffer::BufferType m_Usage = Buffer::BufferType::VERTEX;
    VkIndexType m_IndexType = VK_INDEX_TYPE_NONE_NV;
    VkBuffer m_Buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_Memory = VK_NULL_HANDLE;
    uint32_t m_ElementsCount = 0;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANRESOURCES_H_