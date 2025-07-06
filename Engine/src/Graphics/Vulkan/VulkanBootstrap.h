#ifndef AERO3D_GRAPHICS_VULKAN_VULKANBOOTSTRAP_H_
#define AERO3D_GRAPHICS_VULKAN_VULKANBOOTSTRAP_H_

#include <unordered_set>
#include <vector>

#include <volk.h>

namespace aero3d {

class VulkanGraphicsDevice;

struct DescriptorPool 
{
    VkDescriptorPool pool;
    uint32_t allocations = 0;
    std::unordered_set<VkDescriptorSet> activeSets;
};

class VulkanDescriptorAllocator 
{
public:
    VulkanDescriptorAllocator(VulkanGraphicsDevice* gd);
    ~VulkanDescriptorAllocator();

    VkDescriptorSet Allocate(VkDescriptorSetLayout layout);
    void Free(VkDescriptorSet set);
    void ResetPools();

private:
    DescriptorPool CreatePool();

private:
    VulkanGraphicsDevice* m_GraphicsDevice = nullptr;

    std::vector<DescriptorPool> m_Pools;
    DescriptorPool* m_CurrentPool = nullptr;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANBOOTSTRAP_H_