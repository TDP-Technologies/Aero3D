#ifndef AERO3D_GRAPHICS_RENDERSYSTEM_H_
#define AERO3D_GRAPHICS_RENDERSYSTEM_H_

#include <SDL3/SDL.h>

#include "Utils/Common.h"
#include "Graphics/Context.h"
#include "Graphics/Viewport.h"

namespace aero3d {

class RenderSystem
{
public:
    static void Init();
    static void Shutdown();

    static Ref<Context> CreateContext(SDL_Window* window);
    static Ref<Viewport> CreateViewport(Ref<Context> context, int width, int height);

private:

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_RENDERSYSTEM_H_