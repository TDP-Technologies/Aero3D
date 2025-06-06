#ifndef AERO3D_GRAPHICS_RENDERSYSTEM_H_
#define AERO3D_GRAPHICS_RENDERSYSTEM_H_

#include <SDL3/SDL.h>

#include "Utils/Common.h"

namespace aero3d {

class RenderSystem
{
public:
    static void Init(SDL_Window* window, int height, int width);
    static void Shutdown();

private:

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_RENDERSYSTEM_H_