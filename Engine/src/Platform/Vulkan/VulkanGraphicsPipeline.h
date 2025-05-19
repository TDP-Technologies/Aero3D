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
    VulkanGraphicsPipeline(std::string& vertexShaderPath, std::string& pixelShaderPath);
    ~VulkanGraphicsPipeline();

    virtual void Bind() override;
    virtual void Unbind() override;

private:
    std::vector<uint32_t> CompileGLSL(const std::string& source, shaderc_shader_kind kind, const std::string& name);

    void CreateShaderModules(std::string& vertexShaderPath,
        std::string& pixelShaderPath);
    void CreatePipelineLayout();
    void CreatePipeline();

private:
    VkDevice m_Device;

    VkShaderModule m_VertexShader;
    VkShaderModule m_PixelShader;
    VkPipelineLayout m_Layout;
    VkPipeline m_Pipeline;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_VULKANGRAPHICSPIPELINE_H_