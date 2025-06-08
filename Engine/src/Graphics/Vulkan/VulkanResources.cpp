#include "Graphics/Vulkan/VulkanResources.h"

#include "Utils/Log.h"
#include "IO/VFS.h"

namespace aero3d {

static VkFormat AttributeTypeToVkFormat(Pipeline::AttributeType type)
{
    switch (type)
    {
    case Pipeline::AttributeType::FLOAT:  return VK_FORMAT_R32_SFLOAT;
    case Pipeline::AttributeType::FLOAT2: return VK_FORMAT_R32G32_SFLOAT;
    case Pipeline::AttributeType::FLOAT3: return VK_FORMAT_R32G32B32_SFLOAT;
    case Pipeline::AttributeType::FLOAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;

    case Pipeline::AttributeType::INT:  return VK_FORMAT_R32_SINT;
    case Pipeline::AttributeType::INT2: return VK_FORMAT_R32G32_SINT;
    case Pipeline::AttributeType::INT3: return VK_FORMAT_R32G32B32_SINT;
    case Pipeline::AttributeType::INT4: return VK_FORMAT_R32G32B32A32_SINT;

    case Pipeline::AttributeType::BOOL: return VK_FORMAT_R8_UINT;

    case Pipeline::AttributeType::MAT2: return VK_FORMAT_R32G32_SFLOAT;
    case Pipeline::AttributeType::MAT3: return VK_FORMAT_R32G32B32_SFLOAT;
    case Pipeline::AttributeType::MAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;

    default: return VK_FORMAT_UNDEFINED;
    }
}

static shaderc_shader_kind ShaderTypeToShaderCKind(Pipeline::ShaderType type)
{
    switch (type)
    {
    case Pipeline::ShaderType::VERTEX:  return shaderc_vertex_shader;
    case Pipeline::ShaderType::PIXEL: return shaderc_fragment_shader;

    default: return shaderc_miss_shader;
    }
}

VulkanPipeline::VulkanPipeline(Ref<VulkanContext> context, Ref<VulkanViewport> viewport, Pipeline::Description desc)
    : m_Context(context)
{
    CreatePipelineLayout();
    CreatePipeline(viewport, desc);
}

VulkanPipeline::~VulkanPipeline()
{
    vkDeviceWaitIdle(m_Context->GetDevice());
    vkDestroyPipeline(m_Context->GetDevice(), m_Pipeline, nullptr);
    vkDestroyPipelineLayout(m_Context->GetDevice(), m_Layout, nullptr);
}

std::vector<uint32_t> VulkanPipeline::CompileGLSL(const std::string& source,
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

VkShaderModule VulkanPipeline::CreateShaderModule(Pipeline::Shader& shader)
{
    std::string source = VFS::ReadFile(shader.Path)->ReadString();

    std::vector<uint32_t> spirv = CompileGLSL(source, ShaderTypeToShaderCKind(shader.Type), shader.Path);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    createInfo.codeSize = spirv.size() * sizeof(uint32_t);
    createInfo.pCode = spirv.data();

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    vkCreateShaderModule(m_Context->GetDevice(), &createInfo, nullptr, &shaderModule);

    return shaderModule;
}

void VulkanPipeline::CreatePipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    vkCreatePipelineLayout(m_Context->GetDevice(), &pipelineLayoutInfo, nullptr, &m_Layout);
}

void VulkanPipeline::CreatePipeline(Ref<VulkanViewport> viewport, Pipeline::Description& desc)
{
    VkShaderModule vertexShader = CreateShaderModule(desc.VertexShader);
    VkShaderModule pixelShader = CreateShaderModule(desc.PixelShader);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertexShader;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = pixelShader;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = nullptr;
    viewportState.scissorCount = 1;
    viewportState.pScissors = nullptr;

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
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto attributes = desc.AttribDesc.GetAttributes();

    if (attributes.size())
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = desc.AttribDesc.GetStride();
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(attributes.size());

        for (int i = 0; i < attributes.size(); i++)
        {
            attributeDescriptions[i].binding = 0;
            attributeDescriptions[i].location = i;
            attributeDescriptions[i].format = AttributeTypeToVkFormat(attributes[i].Type);
            attributeDescriptions[i].offset = attributes[i].Offset;
        }

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    }

    VkFormat colorFormat = viewport->GetFormat();

    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;
    renderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = &renderingInfo;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_Layout;
    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.subpass = 0;
    pipelineInfo.pDynamicState = &dynamicState;

    vkCreateGraphicsPipelines(m_Context->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline);
    vkDestroyShaderModule(m_Context->GetDevice(), vertexShader, nullptr);
    vkDestroyShaderModule(m_Context->GetDevice(), pixelShader, nullptr);
}

} // namespace aero3d