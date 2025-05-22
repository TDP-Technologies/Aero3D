#include "Application.h"

#include "Utils/Log.h"
#include "Core/Window.h"
#include "IO/VFS.h"
#include "Graphics/RenderCommand.h"
#include "Event/EventBus.h"

namespace aero3d {

Application::Application()
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
    VertexLayout layout = VertexLayout({
    LayoutElement("inPos", ElementType::FLOAT2),
    LayoutElement("inColor", ElementType::FLOAT3)
        });

    Ref<GraphicsPipeline> pipeline = 
        RenderCommand::CreateGraphicsPipeline(layout, "res/shaders/vertex.glsl", "res/shaders/pixel.glsl");

    RenderCommand::SetClearColor(0.0f, 0.5f, 0.3f, 1.0f);
    RenderCommand::SetViewport(0, 0, 800, 600);

    float vertices[] = {
         0.0f, -0.5f,   1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,   0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,   0.0f, 0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 2
    };

    Ref<VertexBuffer> vb = RenderCommand::CreateVertexBuffer(vertices, 15 * 4);
    Ref<IndexBuffer> ib = RenderCommand::CreateIndexBuffer(indices, 12, 3);

    while (m_IsRunning)
    {
        Window::PollEvents(m_IsRunning, m_Minimized);

        if (!m_Minimized)
        {
            RenderCommand::RecordCommands();

            pipeline->Bind();
            RenderCommand::DrawIndexed(vb, ib);

            RenderCommand::EndCommands();
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