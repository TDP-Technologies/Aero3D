#ifndef AERO3D_CORE_WINDOW_H_
#define AERO3D_CORE_WINDOW_H_

#include <memory>
#include <string>

#include "Utils/Common.h"

class SDL_Window;

namespace aero3d {

enum class GraphicsContextType 
{
    None,
    Vulkan
};

struct WindowInfo
{
    std::string title;
    int width;
    int height;
    GraphicsContextType graphicsContext;
};

class Window
{
public:
    Window(WindowInfo info);
    ~Window();

    void PollEvents(bool& running, bool& minimized);

    SDL_Window* GetSDLWindow();

private:
    SDL_Window* m_Window = nullptr;

};

} // namespace aero3d

#endif // AERO3D_CORE_WINDOW_H_