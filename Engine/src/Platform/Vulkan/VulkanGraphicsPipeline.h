#ifndef AERO3D_PLATFORM_VULKAN_VULKANGRAPHICSPIPELINE_H_
#define AERO3D_PLATFORM_VULKAN_VULKANGRAPHICSPIPELINE_H_

#include <string>
#include <vector>

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>

#include "Graphics/GraphicsPipeline.h"

namespace aero3d {

class VulkanGraphicsPipeline : public GraphicsPipeline
{
public:
    VulkanGraphicsPipeline(VertexLayout& vertexLayout, 
        std::string& vertexShaderPath, std::string& pixelShaderPath);
    ~VulkanGraphicsPipeline();

    virtual void Bind() override;

private:
    std::vector<uint32_t> CompileGLSL(const std::string& source, shaderc_shader_kind kind, const std::string& name);

    void CreateShaderModules(std::string& vertexShaderPath,
        std::string& pixelShaderPath);
    void CreatePipelineLayout();
    void CreatePipeline(VertexLayout& vertexLayout);

private:
    VkDevice m_Device = VK_NULL_HANDLE;

    VkShaderModule m_VertexShader = VK_NULL_HANDLE;
    VkShaderModule m_PixelShader = VK_NULL_HANDLE;
    VkPipelineLayout m_Layout = VK_NULL_HANDLE;
    VkPipeline m_Pipeline = VK_NULL_HANDLE;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_VULKANGRAPHICSPIPELINE_H_