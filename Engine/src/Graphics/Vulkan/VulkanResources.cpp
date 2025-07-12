#include "Graphics/Vulkan/VulkanResources.h"

#include "Graphics/Vulkan/VulkanGraphicsDevice.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "IO/VFS.h"

namespace aero3d {

VulkanDeviceBuffer::VulkanDeviceBuffer(VulkanGraphicsDevice* gd, BufferDesc desc) 
{
    m_GraphicsDevice = gd;
    m_Description = desc;

    size = desc.size;

    VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    if (desc.usage & USAGE_VERTEX)
        usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (desc.usage & USAGE_INDEX)
        usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (desc.usage & USAGE_UNIFORM)
        usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (desc.usage & USAGE_STORAGE)
        usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = desc.size;
    bufferInfo.usage = usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    A3D_CHECK_VKRESULT(vkCreateBuffer(m_GraphicsDevice->device, &bufferInfo, nullptr, &buffer));
    vkGetBufferMemoryRequirements(m_GraphicsDevice->device, buffer, &memoryRequirements);

    bool isStaging = desc.usage & USAGE_STAGING;
    bool hostVisible = isStaging || desc.dynamic;

    VkMemoryPropertyFlags memoryPropertyFlags =
        hostVisible
        ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = m_GraphicsDevice->FindMemoryType(
        memoryRequirements.memoryTypeBits,
        memoryPropertyFlags
    );

    A3D_CHECK_VKRESULT(vkAllocateMemory(m_GraphicsDevice->device, &allocInfo, nullptr, &memory));

    A3D_CHECK_VKRESULT(vkBindBufferMemory(m_GraphicsDevice->device, buffer, memory, 0));
}

VulkanDeviceBuffer::~VulkanDeviceBuffer() 
{
    if (buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(m_GraphicsDevice->device, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
    }
    if (memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_GraphicsDevice->device, memory, nullptr);
        memory = VK_NULL_HANDLE;
    }
}

inline VkFormat ToVkFormat(TextureFormat format) 
{
    switch (format) 
    {
        case TextureFormat::RGBA8:         return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::BGRA8:         return VK_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::RGBA8_SRGB:    return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::BGRA8_SRGB:    return VK_FORMAT_B8G8R8A8_SRGB;
        case TextureFormat::R32FLOAT:      return VK_FORMAT_R32_SFLOAT;
        case TextureFormat::D32FLOAT:      return VK_FORMAT_D32_SFLOAT;
        case TextureFormat::D24S8:         return VK_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::D32S8:         return VK_FORMAT_D32_SFLOAT_S8_UINT;
        default:                           return VK_FORMAT_R8G8B8A8_UNORM;
    }
}

VulkanTexture::VulkanTexture(VulkanGraphicsDevice* gd, TextureDesc desc) 
{
    m_GraphicsDevice = gd;
    m_Description = desc;

    vkFormat = ToVkFormat(desc.format);
    width = desc.width;
    height = desc.height;
    format = desc.format;
    usage = desc.usage;

    VkImageUsageFlags usageFlags = 0;
    switch (desc.usage)
    {
        case TextureUsage::SAMPLED:
            usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            break;
        case TextureUsage::STORAGE:
            usageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            break;
        case TextureUsage::RENDERTARGET:
            usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            break;
        case TextureUsage::DEPTHSTENCIL:
            usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            break;
    }

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = vkFormat;
    imageInfo.extent.width = desc.width;
    imageInfo.extent.height = desc.height;
    imageInfo.extent.depth = desc.depth;
    imageInfo.mipLevels = desc.mipLevels;
    imageInfo.arrayLayers = desc.arrayLayers;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usageFlags;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    A3D_CHECK_VKRESULT(vkCreateImage(m_GraphicsDevice->device, &imageInfo, nullptr, &image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_GraphicsDevice->device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_GraphicsDevice->FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    A3D_CHECK_VKRESULT(vkAllocateMemory(m_GraphicsDevice->device, &allocInfo, nullptr, &memory));

    vkBindImageMemory(m_GraphicsDevice->device, image, memory, 0);
}

VulkanTexture::VulkanTexture(VulkanGraphicsDevice* gd, TextureDesc desc, VkImage existingImage) 
{
    m_GraphicsDevice = gd;
    image = existingImage;
    vkFormat = ToVkFormat(desc.format);
    width = desc.width;
    height = desc.height;
    format = desc.format;
    usage = desc.usage;

    fromExisting = true;
}

VulkanTexture::~VulkanTexture() 
{
    if (!fromExisting)
    {
        vkDeviceWaitIdle(m_GraphicsDevice->device);

        if (image != VK_NULL_HANDLE)
        {
            vkDestroyImage(m_GraphicsDevice->device, image, nullptr);
            image = VK_NULL_HANDLE;
        }
        if (memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(m_GraphicsDevice->device, memory, nullptr);
            memory = VK_NULL_HANDLE;
        }
    }
}

VulkanTextureView::VulkanTextureView(VulkanGraphicsDevice* gd, TextureViewDesc desc)
{
    m_GraphicsDevice = gd;
    m_Description = desc;

    texture = std::static_pointer_cast<VulkanTexture>(desc.texture);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = ToVkFormat(desc.format);

    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = desc.baseMipLevel;
    viewInfo.subresourceRange.levelCount = desc.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = desc.baseArrayLayer;
    viewInfo.subresourceRange.layerCount = desc.arrayLayers;

    A3D_CHECK_VKRESULT(vkCreateImageView(m_GraphicsDevice->device, &viewInfo, nullptr, &imageView));
}

VulkanTextureView::~VulkanTextureView()
{
    if (imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_GraphicsDevice->device, imageView, nullptr);
        imageView = VK_NULL_HANDLE;
    }
}

Ref<Texture> VulkanTextureView::GetTargetTexture()
{
    return texture;
}

VkSamplerAddressMode ToVkSamplerAddressMode(SamplerAddressMode mode)
{
    switch (mode) 
    {
        case SamplerAddressMode::REPEAT:            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddressMode::CLAMP_TO_EDGE:     return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerAddressMode::CLAMP_TO_BORDER:   return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:                                    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

VulkanSampler::VulkanSampler(VulkanGraphicsDevice* gd, SamplerDesc& desc)
{
    m_GraphicsDevice = gd;
    m_Description = desc;

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    samplerInfo.magFilter = (desc.filter == SamplerFilter::LINEAR) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
    samplerInfo.minFilter = (desc.filter == SamplerFilter::LINEAR) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

    samplerInfo.addressModeU = ToVkSamplerAddressMode(desc.addressModeU);
    samplerInfo.addressModeV = ToVkSamplerAddressMode(desc.addressModeV);
    samplerInfo.addressModeW = ToVkSamplerAddressMode(desc.addressModeW);

    samplerInfo.anisotropyEnable = desc.maxAnisotropy > 1.0f ? VK_TRUE : VK_FALSE;
    samplerInfo.maxAnisotropy = desc.maxAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    A3D_CHECK_VKRESULT(vkCreateSampler(m_GraphicsDevice->device, &samplerInfo, nullptr, &sampler));
}

VulkanSampler::~VulkanSampler()
{
    if (sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(m_GraphicsDevice->device, sampler, nullptr);
        sampler = VK_NULL_HANDLE;
    }
}

VulkanFramebuffer::VulkanFramebuffer(VulkanGraphicsDevice* gd, FramebufferDesc desc) 
{
    m_GraphicsDevice = gd;
    m_Description = desc;

    uint32_t targets = desc.colorTargets.size();

    frames.resize(targets);
    imageViews.resize(targets);

    for (size_t i = 0; i < targets; i++) 
    {
        Ref<VulkanTexture> vt = std::static_pointer_cast<VulkanTexture>(desc.colorTargets[i]);
        frames[i] = vt;

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = vt->image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = vt->vkFormat;

        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        A3D_CHECK_VKRESULT(vkCreateImageView(m_GraphicsDevice->device, &viewInfo, nullptr, &imageViews[i]));

        m_GraphicsDevice->TransitionImageLayout(
            vt->image,
            vt->vkFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_ASPECT_COLOR_BIT);
    }

    renderArea = { frames[0]->width, frames[0]->height };

    if (desc.depthTarget)
    {
        depthStencil = std::static_pointer_cast<VulkanTexture>(desc.depthTarget);

        VkImageAspectFlags aspectMask = 0;
        if (depthStencil->format == TextureFormat::D24S8 || depthStencil->format == TextureFormat::D32S8) {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        } else {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = depthStencil->image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthStencil->vkFormat;
        viewInfo.subresourceRange.aspectMask = aspectMask;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        A3D_CHECK_VKRESULT(vkCreateImageView(m_GraphicsDevice->device, &viewInfo, nullptr, &depthStencilImageView));
        
        m_GraphicsDevice->TransitionImageLayout(
            depthStencil->image,
            depthStencil->vkFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            aspectMask);
    }
}

VulkanFramebuffer::~VulkanFramebuffer() 
{
    vkDeviceWaitIdle(m_GraphicsDevice->device);
    for (auto& imageView : imageViews)
    {
        if (imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(m_GraphicsDevice->device, imageView, nullptr);
            imageView = VK_NULL_HANDLE;
        }
    }
    if (depthStencilImageView != nullptr)
    {
        vkDestroyImageView(m_GraphicsDevice->device, depthStencilImageView, nullptr);
        depthStencilImageView = nullptr;
    }
    imageViews.clear();
    frames.clear();
}

static shaderc_shader_kind ShaderStageToShaderCKind(ShaderStages stage)
{
    switch (stage)
    {
    case STAGE_VERTEX:  return shaderc_vertex_shader;
    case STAGE_FRAGMENT: return shaderc_fragment_shader;

    default: return shaderc_miss_shader;
    }
}

VulkanShader::VulkanShader(VulkanGraphicsDevice* gd, ShaderDesc desc) 
{
    m_GraphicsDevice = gd;
    m_Description = desc;

    std::string filePath = desc.path + ".glsl";

    std::string source = VFS::ReadFile(filePath)->ReadString();

    std::vector<uint32_t> spirv = CompileGLSL(source, ShaderStageToShaderCKind(desc.stage), desc.path);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    createInfo.codeSize = spirv.size() * sizeof(uint32_t);
    createInfo.pCode = spirv.data();

    A3D_CHECK_VKRESULT(vkCreateShaderModule(m_GraphicsDevice->device, &createInfo, nullptr, &shaderModule));
}

VulkanShader::~VulkanShader() 
{
    if (shaderModule != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(m_GraphicsDevice->device, shaderModule, nullptr);
        shaderModule = VK_NULL_HANDLE;
    }
}

std::vector<uint32_t> VulkanShader::CompileGLSL(const std::string& source,
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

static VkDescriptorType ToDescriptorType(ResourceKind kind) 
{
    switch (kind) 
    {
        case ResourceKind::UNIFORMBUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case ResourceKind::STORAGEBUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case ResourceKind::TEXTUREREADONLY:
        case ResourceKind::TEXTUREREADONLY_ARRAY: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case ResourceKind::TEXTUREREADWRITE: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case ResourceKind::SAMPLER:
        case ResourceKind::SAMPLER_ARRAY: return VK_DESCRIPTOR_TYPE_SAMPLER;
        case ResourceKind::COMBINED_IMAGE_SAMPLER:
        case ResourceKind::COMBINED_IMAGE_SAMPLER_ARRAY: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

VulkanResourceLayout::VulkanResourceLayout(VulkanGraphicsDevice* gd, ResourceLayoutDesc desc) 
{
    m_GraphicsDevice = gd;
    m_Description = desc;

    bindings = desc.bindings;

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    for (const auto& binding : desc.bindings)
    {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding.binding;
        layoutBinding.descriptorCount = binding.count;
        layoutBinding.descriptorType = ToDescriptorType(binding.kind);
        layoutBinding.stageFlags = 0;

        if (binding.stages & STAGE_VERTEX)
            layoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        if (binding.stages & STAGE_FRAGMENT)
            layoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        if (binding.stages & STAGE_COMPUTE)
            layoutBinding.stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
        if (binding.stages & STAGE_GEOMETRY)
            layoutBinding.stageFlags |= VK_SHADER_STAGE_GEOMETRY_BIT;
        if (binding.stages & STAGE_TESSCONTROL)
            layoutBinding.stageFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        if (binding.stages & STAGE_TESSEVAL)
            layoutBinding.stageFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

        layoutBindings.push_back(layoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings = layoutBindings.data();

    A3D_CHECK_VKRESULT(vkCreateDescriptorSetLayout(m_GraphicsDevice->device, &layoutInfo, nullptr, &descriptorSetLayout));
}

VulkanResourceLayout::~VulkanResourceLayout() 
{
    if (descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(m_GraphicsDevice->device, descriptorSetLayout, nullptr);
        descriptorSetLayout = VK_NULL_HANDLE;
    }
}

VulkanResourceSet::VulkanResourceSet(VulkanGraphicsDevice* gd, ResourceSetDesc desc) 
{
    m_GraphicsDevice = gd;

    Ref<VulkanResourceLayout> vulkanLayout = std::static_pointer_cast<VulkanResourceLayout>(desc.layout);

    descriptorSet = m_GraphicsDevice->descriptorAllocator->Allocate(vulkanLayout->descriptorSetLayout);

    std::vector<VkWriteDescriptorSet> writes;
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkDescriptorImageInfo> imageInfos;
    std::vector<VkDescriptorImageInfo> samplerInfos;

    for (size_t i = 0; i < vulkanLayout->bindings.size(); ++i)
    {
        const auto& binding = vulkanLayout->bindings[i];
        const auto& resource = desc.resources[i];

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptorSet;
        write.dstBinding = binding.binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;

        std::visit([&](auto&& res) 
        {
            using T = std::decay_t<decltype(res)>;

            if constexpr (std::is_same_v<T, Ref<DeviceBuffer>>) 
            {
                bufferInfos.emplace_back();
                PrepareBufferWrite(binding, res.get(), write, bufferInfos.back());
                write.descriptorType = (binding.kind == ResourceKind::UNIFORMBUFFER)
                    ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
                    : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                write.pBufferInfo = &bufferInfos.back();
            } 
            else if constexpr (std::is_same_v<T, Ref<TextureView>>) 
            {
                imageInfos.emplace_back();
                PrepareImageWrite(binding, res.get(), write, imageInfos.back());

                write.descriptorType = (binding.kind == ResourceKind::TEXTUREREADONLY)
                    ? VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
                    : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                write.pImageInfo = &imageInfos.back();
            } 
            else if constexpr (std::is_same_v<T, Ref<Sampler>>) 
            {
                samplerInfos.emplace_back();
                PrepareSamplerWrite(binding, res.get(), write, samplerInfos.back());

                write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                write.pImageInfo = &samplerInfos.back();
            }
            else if constexpr (std::is_same_v<T, std::pair<Ref<TextureView>, Ref<Sampler>>>) 
            {
                imageInfos.emplace_back();
                auto* view = static_cast<VulkanTextureView*>(res.first.get());
                auto* sampler = static_cast<VulkanSampler*>(res.second.get());

                imageInfos.back().imageView = view->imageView;
                imageInfos.back().sampler = sampler->sampler;
                imageInfos.back().imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write.descriptorCount = 1;
                write.pImageInfo = &imageInfos.back();
            }
            else if constexpr (std::is_same_v<T, std::vector<Ref<TextureView>>>)
            {
                for (auto& tex : res) 
                {
                    imageInfos.emplace_back();
                    PrepareImageWrite(binding, tex.get(), write, imageInfos.back());
                }
                write.descriptorType = (binding.kind == ResourceKind::TEXTUREREADONLY || binding.kind == ResourceKind::TEXTUREREADONLY_ARRAY)
                    ? VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
                    : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                write.descriptorCount = static_cast<uint32_t>(res.size());
                write.pImageInfo = imageInfos.data() + (imageInfos.size() - res.size());
            }
            else if constexpr (std::is_same_v<T, std::vector<Ref<Sampler>>>)
            {
                for (auto& samp : res) 
                {
                    samplerInfos.emplace_back();
                    PrepareSamplerWrite(binding, samp.get(), write, samplerInfos.back());
                }
                write.descriptorCount = static_cast<uint32_t>(res.size());
                write.pImageInfo = samplerInfos.data() + (samplerInfos.size() - res.size());
            }
            else if constexpr (std::is_same_v<T, std::vector<std::pair<Ref<TextureView>, Ref<Sampler>>>>) 
            {
                for (auto& [tex, samp] : res) 
                {
                    imageInfos.emplace_back();
                    auto* view = static_cast<VulkanTextureView*>(tex.get());
                    auto* sampler = static_cast<VulkanSampler*>(samp.get());

                    imageInfos.back().imageView = view->imageView;
                    imageInfos.back().sampler = sampler->sampler;
                    imageInfos.back().imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                }

                write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write.descriptorCount = static_cast<uint32_t>(res.size());
                write.pImageInfo = imageInfos.data() + (imageInfos.size() - res.size());
            }
        }, resource);

        writes.push_back(write);
    }

    vkUpdateDescriptorSets(m_GraphicsDevice->device,
        static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

VulkanResourceSet::~VulkanResourceSet() 
{
    if (descriptorSet != VK_NULL_HANDLE)
    {
        m_GraphicsDevice->descriptorAllocator->Free(descriptorSet);
        descriptorSet = VK_NULL_HANDLE;
    }
}

void VulkanResourceSet::PrepareBufferWrite(const ResourceBinding& binding, void* resource,
    VkWriteDescriptorSet& write, VkDescriptorBufferInfo& bufferInfo)
{
    auto* buffer = static_cast<VulkanDeviceBuffer*>(resource);
    bufferInfo.buffer = buffer->buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;
}

void VulkanResourceSet::PrepareImageWrite(const ResourceBinding& binding, void* resource,
    VkWriteDescriptorSet& write, VkDescriptorImageInfo& imageInfo)
{
    auto* view = static_cast<VulkanTextureView*>(resource);
    imageInfo.imageView = view->imageView;
    imageInfo.imageLayout = (binding.kind == ResourceKind::TEXTUREREADONLY || binding.kind == ResourceKind::TEXTUREREADONLY_ARRAY)
        ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        : VK_IMAGE_LAYOUT_GENERAL;
}

void VulkanResourceSet::PrepareSamplerWrite(const ResourceBinding& binding, void* resource,
    VkWriteDescriptorSet& write, VkDescriptorImageInfo& imageInfo)
{
    auto* sampler = static_cast<VulkanSampler*>(resource);
    imageInfo.sampler = sampler->sampler;
}

static VkPolygonMode ToVkPolygonMode(PolygonMode mode) 
{
    switch (mode) 
    {
        case PolygonMode::FILL: return VK_POLYGON_MODE_FILL;
        case PolygonMode::LINE: return VK_POLYGON_MODE_LINE;
        case PolygonMode::POINT: return VK_POLYGON_MODE_POINT;
        default: return VK_POLYGON_MODE_FILL;
    }
}

static VkCullModeFlags ToVkCullMode(CullMode mode)
{
    switch (mode) 
    {
        case CullMode::NONE: return VK_CULL_MODE_NONE;
        case CullMode::FRONT: return VK_CULL_MODE_FRONT_BIT;
        case CullMode::BACK: return VK_CULL_MODE_BACK_BIT;
        default: return VK_CULL_MODE_BACK_BIT;
    }
}

static VkFrontFace ToVkFrontFace(FrontFace face) 
{
    switch (face) 
    {
        case FrontFace::CLOCKWISE: return VK_FRONT_FACE_CLOCKWISE;
        case FrontFace::COUNTERCLOCKWISE: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        default: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
}

static VkFormat VertexFormatToVkFormat(VertexFormat type)
{
    switch (type)
    {
        case VertexFormat::FLOAT:  return VK_FORMAT_R32_SFLOAT;
        case VertexFormat::FLOAT2: return VK_FORMAT_R32G32_SFLOAT;
        case VertexFormat::FLOAT3: return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexFormat::FLOAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;

        case VertexFormat::INT:  return VK_FORMAT_R32_SINT;
        case VertexFormat::INT2: return VK_FORMAT_R32G32_SINT;
        case VertexFormat::INT3: return VK_FORMAT_R32G32B32_SINT;
        case VertexFormat::INT4: return VK_FORMAT_R32G32B32A32_SINT;

        case VertexFormat::BOOL: return VK_FORMAT_R8_UINT;

        case VertexFormat::MAT2: return VK_FORMAT_R32G32_SFLOAT;
        case VertexFormat::MAT3: return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexFormat::MAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;

        default: return VK_FORMAT_UNDEFINED;
    }
}

VulkanPipeline::VulkanPipeline(VulkanGraphicsDevice* gd, PipelineDesc desc) 
{
    m_GraphicsDevice = gd;
    m_Description = desc;

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    auto AddShaderStage = [&](Ref<Shader> shader, VkShaderStageFlagBits stageFlag) 
    {
        auto vkShader = std::static_pointer_cast<VulkanShader>(shader);
        VkPipelineShaderStageCreateInfo shaderStage{};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = stageFlag;
        shaderStage.module = vkShader->shaderModule;
        shaderStage.pName = "main";
        shaderStages.push_back(shaderStage);
    };

    if (desc.vertexShader)
        AddShaderStage(desc.vertexShader, VK_SHADER_STAGE_VERTEX_BIT);
    if (desc.fragmentShader)
        AddShaderStage(desc.fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    for (const auto& b : desc.vertexLayout.bindings) 
    {
        VkVertexInputBindingDescription bindingDesc = {};
        bindingDesc.binding = b.binding;
        bindingDesc.stride = b.stride;
        bindingDesc.inputRate = b.perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions.push_back(bindingDesc);
    }

    for (const auto& a : desc.vertexLayout.attributes) 
    {
        VkVertexInputAttributeDescription attrDesc = {};
        attrDesc.location = a.location;
        attrDesc.binding = a.binding;
        attrDesc.offset = a.offset;
        attrDesc.format = VertexFormatToVkFormat(a.format);
        attributeDescriptions.push_back(attrDesc);
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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
    rasterizer.polygonMode = ToVkPolygonMode(desc.polygonMode);
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = ToVkCullMode(desc.cullMode);
    rasterizer.frontFace = ToVkFrontFace(desc.frontFace);
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = desc.depthTest ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = desc.depthWrite ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    auto vkLayout = std::static_pointer_cast<VulkanResourceLayout>(desc.resourceLayout);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &vkLayout->descriptorSetLayout;

    A3D_CHECK_VKRESULT(vkCreatePipelineLayout(m_GraphicsDevice->device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &m_GraphicsDevice->swapchain->imageFormat;
    renderingInfo.depthAttachmentFormat = m_GraphicsDevice->swapchain->depthStencil->vkFormat;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates = dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = &renderingInfo;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.subpass = 0;

    A3D_CHECK_VKRESULT(vkCreateGraphicsPipelines(m_GraphicsDevice->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));
}

VulkanPipeline::~VulkanPipeline() 
{
    if (pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(m_GraphicsDevice->device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
    if (pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(m_GraphicsDevice->device, pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
    }
}

} // namespace aero3d
