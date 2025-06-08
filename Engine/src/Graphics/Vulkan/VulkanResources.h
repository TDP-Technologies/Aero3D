#ifndef AERO3D_GRAPHICS_VULKAN_VULKANRESOURCES_H_
#define AERO3D_GRAPHICS_VULKAN_VULKANRESOURCES_H_

#include <volk.h>
#include <shaderc/shaderc.hpp>

#include "Utils/Common.h"
#include "Graphics/Resources.h"
#include "Graphics/Vulkan/VulkanContext.h"

namespace aero3d {

class VulkanPipeline : public Pipeline
{
public:
    VulkanPipeline(Ref<VulkanContext> context, Pipeline::Description desc);
    ~VulkanPipeline();

    VkPipeline GetPipeline() { return m_Pipeline; }

private:
    std::vector<uint32_t> CompileGLSL(const std::string& source, shaderc_shader_kind kind, const std::string& name);

    VkShaderModule CreateShaderModule(Pipeline::Shader& shader);
    void CreatePipelineLayout();
    void CreatePipeline(Pipeline::Description& vertexLayout);

private:
    Ref<VulkanContext> m_Context = nullptr;
    VkPipelineLayout m_Layout = VK_NULL_HANDLE;
    VkPipeline m_Pipeline = VK_NULL_HANDLE;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANRESOURCES_H_