#ifndef AERO3D_GRAPHICS_RENDERER_H_
#define AERO3D_GRAPHICS_RENDERER_H_

namespace aero3d {

struct RenderCommand
{

};

class Renderer
{
public:
    virtual ~Renderer() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;

    virtual void SubmitCommand(RenderCommand command) = 0;
    virtual void ResizeBuffers() = 0;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_RENDERER_H_