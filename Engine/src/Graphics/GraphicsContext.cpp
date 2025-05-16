#include "Graphics/GraphicsContext.h"

#include <memory>
#include <string.h>

#include "Core/Configuration.h"
#include "Utils/Assert.h"

namespace aero3d {

std::unique_ptr<GraphicsContext> GraphicsContext::Create(const char* api)
{
    return nullptr;
}

} // namespace aero3d