#ifndef AERO3D_PLATFORM_VULKAN_VULKANGRAPHICSPIPELINE_H_
#define AERO3D_PLATFORM_VULKAN_VULKANGRAPHICSPIPELINE_H_

#include <string>

#include <vulkan/vulkan.h>

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
    void CreatePipelineLayout();
    void CreatePipeline();

private:
    VkDevice m_Device;

    VkPipelineLayout m_Layout;
    VkPipeline m_Pipeline;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_VULKANGRAPHICSPIPELINE_H_