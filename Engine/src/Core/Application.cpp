#include "Application.h"

#include "Utils/Log.h"
#include "Core/Window.h"
#include "IO/VFS.h"
#include "Event/EventBus.h"
#include "Graphics/RenderSystem.h"

namespace aero3d {

bool Application::Init()
{
    LogMsg("Application Initialize.");

    VFS::Init();
    EventBus::Init();
    Window::Init("Aero3D", 800, 600);
    RenderSystem::Init(Window::GetSDLWindow(), 800, 600);

    SubscribeOnEvents();

    VFS::Mount("", "Sandbox/");

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
            renderer->BeginFrame();

            renderer->EndFrame();
        }
    }
}

void Application::Shutdown()
{
    LogMsg("Application Shutdown.");

    RenderSystem::Shutdown();
    Window::Shutdown();
    EventBus::Shutdown();
    VFS::Shutdown();
}

void Application::SubscribeOnEvents()
{
    EventBus::Subscribe(typeid(WindowResizeEvent), [&](Event& event) 
    {
        WindowResizeEvent& windowResizeEvent = static_cast<WindowResizeEvent&>(event);

    });
}

} // namespace aero3d