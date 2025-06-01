#include "Platform/Vulkan/Internal/VulkanDescriptors.h"

#include "Platform/Vulkan/Internal/VulkanCore.h"
#include "Platform/Vulkan/Internal/VulkanUtils.h"

namespace aero3d {

void VulkanDescriptorSetLayout::AddBinding(uint32_t binding, VkDescriptorType type,
                                           VkShaderStageFlags stageFlags, uint32_t count)
{
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = type;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    m_Bindings[binding] = layoutBinding;
}

void VulkanDescriptorSetLayout::Init()
{
    m_Device = g_VulkanCore->GetDeviceHandle();

    std::vector<VkDescriptorSetLayoutBinding> bindingsVec;
    for (auto &[key, value] : m_Bindings)
        bindingsVec.push_back(value);

    CreateDescriptorSetLayout(m_Device, m_DescriptorSetLayout, bindingsVec.data(), bindingsVec.size());
}

void VulkanDescriptorSetLayout::Shutdown()
{
    vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
}

void VulkanDescriptorPool::AddPoolSize(VkDescriptorType type, uint32_t count) 
{
    m_PoolSizes.push_back({type, count});
}

void VulkanDescriptorPool::SetMaxSets(uint32_t count) 
{
    m_MaxSets = count;
}

void VulkanDescriptorPool::SetFlags(VkDescriptorPoolCreateFlags flags) 
{
    m_Flags = flags;
}

void VulkanDescriptorPool::Init() 
{
    m_Device = g_VulkanCore->GetDeviceHandle();

    CreateDescriptorPool(m_Device, m_DescriptorPool, m_PoolSizes.data(), m_PoolSizes.size(),
        m_MaxSets, m_Flags);
}

void VulkanDescriptorPool::Shutdown() 
{
    vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
}

void VulkanDescriptorPool::Allocate(VkDescriptorSetLayout layout, VkDescriptorSet &set) 
{
    AllocateDescriptorSet(m_Device, m_DescriptorPool, layout, set);
}

void VulkanDescriptorPool::Free(std::vector<VkDescriptorSet> &sets) 
{
    if (!sets.empty()) 
    {
        vkFreeDescriptorSets(m_Device, m_DescriptorPool,
            static_cast<uint32_t>(sets.size()), sets.data());
    }
}

void VulkanDescriptorPool::Reset() 
{
    vkResetDescriptorPool(m_Device, m_DescriptorPool, 0);
}

void VulkanDescriptorWriter::Init(VulkanDescriptorSetLayout* layout, VulkanDescriptorPool* pool)
{
    m_SetLayout = layout;
    m_Pool = pool;
}

VulkanDescriptorWriter &VulkanDescriptorWriter::WriteBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo) 
{
    auto &desc = m_SetLayout->m_Bindings[binding];

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstBinding = binding;
    write.descriptorType = desc.descriptorType;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    m_Writes.push_back(write);
    return *this;
}

VulkanDescriptorWriter &VulkanDescriptorWriter::WriteImage(uint32_t binding, VkDescriptorImageInfo *imageInfo)
{
    auto &desc = m_SetLayout->m_Bindings[binding];

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstBinding = binding;
    write.descriptorType = desc.descriptorType;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    m_Writes.push_back(write);
    return *this;
}

void VulkanDescriptorWriter::Build(VkDescriptorSet &set) 
{
    m_Pool->Allocate(m_SetLayout->GetHandle(), set);
    Overwrite(set);
}

void VulkanDescriptorWriter::Overwrite(VkDescriptorSet &set) 
{
    for (auto &write : m_Writes)
    {
        write.dstSet = set;
    }

    vkUpdateDescriptorSets(m_Pool->m_Device, static_cast<uint32_t>(m_Writes.size()), m_Writes.data(), 0, nullptr);
}

} // namespace aero3d