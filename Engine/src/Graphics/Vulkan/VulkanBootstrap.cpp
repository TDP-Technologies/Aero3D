#include "Graphics/Vulkan/VulkanBootstrap.h"

#include "Graphics/Vulkan/VulkanGraphicsDevice.h"
#include "Graphics/Vulkan/VulkanUtils.h"

namespace aero3d {

VulkanDescriptorAllocator::VulkanDescriptorAllocator(VulkanGraphicsDevice* gd)
{
    m_GraphicsDevice = gd;
}

VulkanDescriptorAllocator::~VulkanDescriptorAllocator() 
{
    for (auto& pool : m_Pools)
    {
        vkDestroyDescriptorPool(m_GraphicsDevice->device, pool.pool, nullptr);
    }
}

VkDescriptorSet VulkanDescriptorAllocator::Allocate(VkDescriptorSetLayout layout) 
{
    if (!m_CurrentPool || m_CurrentPool->allocations >= 128) 
    {
        DescriptorPool newPool = CreatePool();
        m_Pools.push_back(newPool);
        m_CurrentPool = &m_Pools.back();
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_CurrentPool->pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet set;
    A3D_CHECK_VKRESULT(vkAllocateDescriptorSets(m_GraphicsDevice->device, &allocInfo, &set));

    m_CurrentPool->allocations++;
    m_CurrentPool->activeSets.insert(set);
    return set;
}

void VulkanDescriptorAllocator::Free(VkDescriptorSet set) 
{
    for (auto& pool : m_Pools) 
    {
        if (pool.activeSets.find(set) != pool.activeSets.end()) 
        {
            vkFreeDescriptorSets(m_GraphicsDevice->device, pool.pool, 1, &set);
            pool.activeSets.erase(set);
            pool.allocations--;
            return;
        }
    }
}

void VulkanDescriptorAllocator::ResetPools() 
{
    for (auto& pool : m_Pools) 
    {
        vkResetDescriptorPool(m_GraphicsDevice->device, pool.pool, 0);
        pool.allocations = 0;
        pool.activeSets.clear();
    }
    m_CurrentPool = m_Pools.empty() ? nullptr : &m_Pools.front();
}

DescriptorPool VulkanDescriptorAllocator::CreatePool() 
{
    std::vector<VkDescriptorPoolSize> poolSizes = 
    {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 64 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 32 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 64 },
        { VK_DESCRIPTOR_TYPE_SAMPLER, 64 }
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 128;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    DescriptorPool result{};
    A3D_CHECK_VKRESULT(vkCreateDescriptorPool(m_GraphicsDevice->device, &poolInfo, nullptr, &result.pool));

    return result;
}

} // namespace aero3d