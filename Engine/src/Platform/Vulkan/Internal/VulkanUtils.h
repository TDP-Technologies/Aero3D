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
void CreateInstance(VkInstance* pInstance);

void CreateSurface(VkInstance instance, SDL_Window* window, VkSurfaceKHR* pSurface);

void CreateRenderPass(VkDevice device, VkFormat format, VkRenderPass* pRenderPass);

void CreateFramebuffer(VkDevice device, VkImageView imageView, VkExtent2D extent, 
    VkRenderPass renderPass, VkFramebuffer* pFrameBuffer);

void CreateCommandPool(VkDevice device, uint32_t queueIndex, VkCommandPool* pCommandPool);

void CreateCommandBuffers(VkDevice device, VkCommandPool commandPool,
    uint32_t count, VkCommandBuffer* pCommandBuffers);

void CreateSemaphore(VkDevice device, VkSemaphore* pSemaphore);

void CreateFence(VkDevice device, VkFence* pFence);

///////////////////////////////////////////// SwapChain /////////////////////////////////////////////

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

void CreateImageView(VkDevice device, VkImage image, VkFormat imageFormat, VkImageView* pImageView);

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANUTILS_H_