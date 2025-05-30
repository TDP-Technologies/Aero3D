#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANUTILS_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANUTILS_H_

#include <vector>

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>

#include "Utils/Log.h"

#define A3D_CHECK_VKRESULT(res) \
    if (res != VK_SUCCESS) { \
        LogErr(ERROR_INFO, "VkResult Failed: %s (%d)", VkResultToString(res), res); \
    }

namespace aero3d {

////////////////////////////////////////////// Common ///////////////////////////////////////////////
const char* VkResultToString(VkResult result);

////////////////////////////////////////////// Core /////////////////////////////////////////////////
void CreateInstance(VkInstance& pInstance);

void CreateSurface(VkInstance instance, SDL_Window* window, VkSurfaceKHR& surface);

void CreateRenderPass(VkDevice device, VkFormat format, VkRenderPass& renderPass);

void CreateFramebuffer(VkDevice device, VkImageView imageView, VkExtent2D extent, 
    VkRenderPass renderPass, VkFramebuffer& frameBuffer);

void CreateCommandPool(VkDevice device, uint32_t queueIndex, VkCommandPool& commandPool);

void CreateCommandBuffers(VkDevice device, VkCommandPool commandPool,
    uint32_t count, VkCommandBuffer* pCommandBuffers);

void CreateSemaphore(VkDevice device, VkSemaphore& semaphore);

void CreateFence(VkDevice device, VkFence& fence);

void BeginCommandBuffer(VkCommandBuffer commandBuffer);

///////////////////////////////////////////// SwapChain /////////////////////////////////////////////

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

void CreateImageView(VkDevice device, VkImage image, VkFormat imageFormat, VkImageView& imageView);

////////////////////////////////////////////// Buffer /////////////////////////////////////////////////

uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, 
    VkPhysicalDevice physicalDevice);

void CreateBuffer(VkDevice device, VkBufferUsageFlags usage, size_t size, VkBuffer& buffer,
    VkMemoryPropertyFlags properties, VkPhysicalDevice physDevice, VkDeviceMemory& deviceMemory);

void WriteBufferMemory(VkDevice device, VkDeviceMemory memory, void* data, size_t size);

void PrepareStagingBuffer(VkDevice device, VkBuffer& buffer, VkPhysicalDevice physDevice,
    VkDeviceMemory& memory, void* data, size_t size);

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANUTILS_H_