#ifndef AERO3D_CORE_WINDOW_H_
#define AERO3D_CORE_WINDOW_H_

#include <memory>

#include "Utils/Common.h"

class SDL_Window;

namespace aero3d {

class Window
{
public:
    Window(const char* title, int width, int height);
    ~Window();

    void PollEvents(bool& running, bool& minimized);

    SDL_Window* GetSDLWindow();

private:
    SDL_Window* m_Window = nullptr;

};

} // namespace aero3d

#endif // AERO3D_CORE_WINDOW_H_