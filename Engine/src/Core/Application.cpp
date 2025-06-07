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
    RenderSystem::Init();

    SubscribeOnEvents();

    VFS::Mount("", "Sandbox/");

    m_IsRunning = true;

    return true;
}

void Application::Run()
{
    Ref<Context> context = RenderSystem::CreateContext(Window::GetSDLWindow());
    Ref<Viewport> viewport = RenderSystem::CreateViewport(context, 800, 600);

    while (m_IsRunning)
    {
        Window::PollEvents(m_IsRunning, m_Minimized);

        if (!m_Minimized)
        {
            viewport->SwapBuffers();
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