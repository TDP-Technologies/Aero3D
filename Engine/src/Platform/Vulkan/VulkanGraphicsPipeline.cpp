#include "Platform/Vulkan/VulkanGraphicsPipeline.h"

#include <vector>

#include "Platform/Vulkan/Internal/VulkanCore.h"
#include "Platform/Vulkan/Internal/VulkanUtils.h"
#include "Utils/Log.h"
#include "IO/VFS.h"

namespace aero3d {

VulkanGraphicsPipeline::VulkanGraphicsPipeline(std::string& vertexShaderPath,
    std::string& pixelShaderPath)
    : m_Pipeline(VK_NULL_HANDLE), m_Device(VK_NULL_HANDLE)
{
    m_Device = g_VulkanCore->GetDevice().GetDevice();

    CreateShaderModules(vertexShaderPath, pixelShaderPath);
    CreatePipelineLayout();
    CreatePipeline();
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
    vkDeviceWaitIdle(m_Device);
    vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    vkDestroyPipelineLayout(m_Device, m_Layout, nullptr);
    vkDestroyShaderModule(m_Device, m_VertexShader, nullptr);
    vkDestroyShaderModule(m_Device, m_PixelShader, nullptr);
}

void VulkanGraphicsPipeline::Bind()
{
    vkCmdBindPipeline(g_VulkanCore->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
}

void VulkanGraphicsPipeline::Unbind()
{

}

std::vector<uint32_t> VulkanGraphicsPipeline::CompileGLSL(const std::string& source,
    shaderc_shader_kind kind, const std::string& name)
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, kind, name.c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        LogErr(ERROR_INFO, "Failed to compile Vulkan Shader");
    }

    return { result.cbegin(), result.cend() };
}

void VulkanGraphicsPipeline::CreateShaderModules(std::string& vertexShaderPath,
    std::string& pixelShaderPath)
{
    std::string vertexSource = VFS::ReadFile(vertexShaderPath)->ReadString();
    std::string pixelSource = VFS::ReadFile(pixelShaderPath)->ReadString();

    std::vector<uint32_t> spirvVertex = CompileGLSL(vertexSource, shaderc_vertex_shader, vertexShaderPath);
    std::vector<uint32_t> spirvPixel = CompileGLSL(pixelSource, shaderc_fragment_shader, pixelShaderPath);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    createInfo.codeSize = spirvVertex.size() * sizeof(uint32_t);
    createInfo.pCode = spirvVertex.data();

    A3D_CHECK_VKRESULT(vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_VertexShader));

    createInfo.codeSize = spirvPixel.size() * sizeof(uint32_t);
    createInfo.pCode = spirvPixel.data();

    A3D_CHECK_VKRESULT(vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_PixelShader));
}

void VulkanGraphicsPipeline::CreatePipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    A3D_CHECK_VKRESULT(vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_Layout));
}

void VulkanGraphicsPipeline::CreatePipeline()
{
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = m_VertexShader;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = m_PixelShader;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { 1, 1 };

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = nullptr;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_Layout;
    pipelineInfo.renderPass = g_VulkanCore->GetRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.pDynamicState = &dynamicState;

    A3D_CHECK_VKRESULT(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline));
}

} // namespace aero3d