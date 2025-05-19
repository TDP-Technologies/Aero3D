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

    A3D_CHECK_INIT(VFS::Init());
    A3D_CHECK_INIT(EventBus::Init());
    A3D_CHECK_INIT(Window::Init("Aero3D", 800,
        600, "Vulkan"));
    A3D_CHECK_INIT(RenderCommand::Init("Vulkan"));

    SubscribeOnEvents();

    m_IsRunning = true;

    return true;
}

void Application::Run()
{
    std::shared_ptr<GraphicsPipeline> pipeline = 
        RenderCommand::CreateGraphicsPipeline("res/shaders/vertex.glsl", "res/shaders/pixel.glsl");

    RenderCommand::SetClearColor(1.0f, 0.5f, 0.3f, 1.0f);

    while (m_IsRunning)
    {
        Window::PollEvents(m_IsRunning, m_Minimized);

        if (!m_Minimized)
        {
            pipeline->Bind();
        }

        Window::SwapBuffers();
    }
}

void Application::Shutdown()
{
    LogMsg("Application Shutdown.");

    RenderCommand::Shutdown();
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