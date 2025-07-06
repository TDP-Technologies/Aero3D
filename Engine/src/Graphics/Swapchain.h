#ifndef AERO3D_GRAPHICS_SWAPCHAIN_H_
#define AERO3D_GRAPHICS_SWAPCHAIN_H_

#include "Graphics/Resources.h"

namespace aero3d {

class Swapchain 
{
public:
    virtual ~Swapchain() = default;

    virtual void Resize() = 0;

    virtual Ref<Framebuffer> GetFramebuffer() = 0;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_SWAPCHAIN_H_