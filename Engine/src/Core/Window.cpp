#include "Core/Window.h"

#include <memory>

#include "Utils/Common.h"
#include "Utils/Log.h"
#include "Event/EventBus.h"

namespace aero3d {

SDL_Window* Window::s_Window = nullptr;

void Window::Init(const char* title, int width, int height)
{
    LogMsg("Window Initialize.");

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) 
    {
        LogErr(ERROR_INFO, "SDL Init Failed. SDL Error: %s", SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    s_Window = SDL_CreateWindow(title,
        width, height,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    if (!s_Window) 
    {
        LogErr(ERROR_INFO, "SDL Create Window Failed. SDL Error: %s", SDL_GetError());
    }
}

void Window::Shutdown()
{
    LogMsg("Window Shutdown.");

    if (s_Window) 
    {
        SDL_DestroyWindow(s_Window);
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Window::PollEvents(bool& running, bool& minimized)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        switch (event.type) 
        {
        case SDL_EVENT_QUIT:
        {
            running = false;
            break;
        }
        case SDL_EVENT_WINDOW_MINIMIZED:
        {
            minimized = true;
            break;
        }
        case SDL_EVENT_WINDOW_RESTORED:
        {
            minimized = false;
            break;
        }
        case SDL_EVENT_WINDOW_RESIZED:
        {
            WindowResizeEvent windowResizeEvent(event.window.data1, event.window.data2);
            EventBus::Publish(windowResizeEvent);
            break;
        }
        default:
        {
            break;
        }
        }
    }
}

SDL_Window* Window::GetSDLWindow()
{
    return s_Window;
}

} // namespace aero3d