#include "Application.h"

#include <random>

#include "Utils/Log.h"
#include "Core/Window.h"
#include "IO/VFS.h"
#include "Event/EventBus.h"

namespace aero3d {

bool Application::Init()
{
    LogMsg("Application Initialize.");

    A3D_CHECK_INIT(VFS::Init());
    A3D_CHECK_INIT(EventBus::Init());
    A3D_CHECK_INIT(Window::Init("Aero3D", 800,
        600, "Vulkan"));

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

        }
    }
}

void Application::Shutdown()
{
    LogMsg("Application Shutdown.");

    Window::Shutdown();
    EventBus::Shutdown();
    VFS::Shutdown();
}

void Application::SubscribeOnEvents()
{
    EventBus::Subscribe(typeid(WindowResizeEvent), [&](Event& event) {
        WindowResizeEvent& windowResizeEvent = static_cast<WindowResizeEvent&>(event);

    });
}

} // namespace aero3d