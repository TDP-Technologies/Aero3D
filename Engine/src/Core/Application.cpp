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

    VFS::Mount("", "Sandbox/");

    m_IsRunning = true;

    return true;
}

void Application::Run()
{
    Ref<Context> context = RenderSystem::CreateContext(Window::GetSDLWindow());
    Ref<Viewport> viewport = RenderSystem::CreateViewport(context, 800, 600);
    Ref<CommandBuffer> commandBuffer = RenderSystem::CreateCommandBuffer(context, viewport);

    EventBus::Subscribe(typeid(WindowResizeEvent), [&](Event& event) 
    {
        WindowResizeEvent& windowResizeEvent = static_cast<WindowResizeEvent&>(event);
        viewport->Resize(windowResizeEvent.GetWidth(), windowResizeEvent.GetHeight());
    });

    Pipeline::InputAttributeDescription pipelineAttributeDesc = Pipeline::InputAttributeDescription({
        //Pipeline::Attribute("inPos", Pipeline::AttributeType::FLOAT2),
        //Pipeline::Attribute("inColor", Pipeline::AttributeType::FLOAT3)
    });

    Pipeline::Description pipelineDescription { 
        pipelineAttributeDesc, 
        { Pipeline::ShaderType::VERTEX, "res/shaders/vertex.glsl" },
        { Pipeline::ShaderType::PIXEL, "res/shaders/pixel.glsl" }
    };

    Ref<Pipeline> pipeline = RenderSystem::CreatePipeline(context, pipelineDescription);

    while (m_IsRunning)
    {
        Window::PollEvents(m_IsRunning, m_Minimized);

        if (!m_Minimized)
        {
            commandBuffer->Record();

            commandBuffer->BindPipeline(pipeline);

            commandBuffer->End();

            commandBuffer->Execute();

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

} // namespace aero3d