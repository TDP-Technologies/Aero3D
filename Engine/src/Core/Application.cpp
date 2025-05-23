#include "Application.h"

#include "Utils/Log.h"
#include "Core/Window.h"
#include "IO/VFS.h"
#include "Graphics/RenderCommand.h"
#include "Event/EventBus.h"

namespace aero3d {

Application::Application()
    : m_IsRunning(false), m_Minimized(false)
{
}

Application::~Application()
{
}

bool Application::Init()
{
    LogMsg("Application Initialize.");

    if (!VFS::Init())
    {
        return false;
    }

    if (!EventBus::Init())
    {
        return false;
    }

    if (!Window::Init("Aero3D", 800,
        600, "OpenGL"))
    {
        return false;
    }

    /*
    if (!RenderCommand::Init("OpenGL"))
    {
        return false;
    }
    */

    SubscribeOnEvents();

    m_IsRunning = true;

    return true;
}

void Application::Run()
{
    while (m_IsRunning)
    {
        Window::PollEvents(m_IsRunning, m_Minimized);

        if (!m_Minimized)
        {

        }

        Window::SwapBuffers();
    }
}

void Application::Shutdown()
{
    LogMsg("Application Shutdown.");

    //RenderCommand::Shutdown();
    Window::Shutdown();
    EventBus::Shutdown();
    VFS::Shutdown();
}

void Application::SubscribeOnEvents()
{
    EventBus::Subscribe(typeid(WindowResizeEvent), [&](Event& event) {
        WindowResizeEvent& windowResizeEvent = static_cast<WindowResizeEvent&>(event);

        int width = std::max(1, windowResizeEvent.GetWidth());
        int height = std::max(1, windowResizeEvent.GetHeight());

        //RenderCommand::SetViewport(0, 0, width, height);
    });
}

} // namespace aero3d