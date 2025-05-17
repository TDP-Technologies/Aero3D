#include "Graphics/GraphicsContext.h"

#include <memory>
#include <string.h>

#include "Core/Configuration.h"
#include "Utils/Assert.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace aero3d {

std::unique_ptr<GraphicsContext> GraphicsContext::Create(const char* api)
{
    return std::make_unique<VulkanContext>();
}

} // namespace aero3d