#ifndef AERO3D_GRAPHICS_VIEWPORT_H_
#define AERO3D_GRAPHICS_VIEWPORT_H_

namespace aero3d {

class Viewport
{
public:
    virtual ~Viewport() = default;

    virtual void SwapBuffers() = 0;
    virtual void Resize() = 0;

};
    
} // namespace aero3d

#endif // AERO3D_GRAPHICS_VIEWPORT_H_