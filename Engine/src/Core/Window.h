#ifndef AERO3D_CORE_WINDOW_H_
#define AERO3D_CORE_WINDOW_H_

#include <memory>

#include "SDL3/SDL.h"

#include "Utils/Common.h"

namespace aero3d {

class Window
{
public:
    static void Init(const char* title, int width, int height);
    static void Shutdown();

    static void PollEvents(bool& running, bool& minimized);

    static SDL_Window* GetSDLWindow();

private:
    static SDL_Window* s_Window;

};

} // namespace aero3d

#endif // AERO3D_CORE_WINDOW_H_