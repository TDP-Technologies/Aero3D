#ifndef AERO3D_GRAPHICS_RENDERER_H_
#define AERO3D_GRAPHICS_RENDERER_H_

#include <SDL3/SDL.h>

namespace aero3d {

struct RenderCommand
{

};

class Renderer
{
public:
    virtual ~Renderer() = default;

    virtual void Init(SDL_Window* window, int width, int height) = 0;
    virtual void Shutdown() = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;

    virtual void SubmitCommand(RenderCommand command) = 0;
    virtual void ResizeBuffers() = 0;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_RENDERER_H_