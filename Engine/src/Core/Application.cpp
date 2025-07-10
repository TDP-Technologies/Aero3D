#include "Application.h"

#include "Utils/Log.h"
#include "IO/VFS.h"
#include "Event/EventBus.h"

#include "Graphics/Vulkan/VulkanGraphicsDevice.h"

namespace aero3d {

Application Application::s_Application;

bool Application::Init()
{
    LogMsg("Application Initialize.");

    VFS::Mount("", "Sandbox/");

    m_Window = new Window("Aero3D", 800, 600);
    m_GraphicsDevice = new VulkanGraphicsDevice(static_cast<SDL_Window*>(m_Window->GetSDLWindow()));
    m_ResourceManager = new ResourceManager(m_GraphicsDevice, m_GraphicsDevice->GetResourceFactory());
    m_Scene = new Scene();
    m_RenderSystem = new RenderSystem(m_GraphicsDevice, m_GraphicsDevice->GetResourceFactory());

    m_IsRunning = true;

    return true;
}

void Application::Run()
{
    EventBus::Subscribe(typeid(WindowResizeEvent), [&](Event& event) 
    {
        WindowResizeEvent& windowResizeEvent = static_cast<WindowResizeEvent&>(event);
        m_GraphicsDevice->GetSwapchain()->Resize();
    });

    uint64_t previousTicks = SDL_GetPerformanceCounter();
    double frequency = static_cast<double>(SDL_GetPerformanceFrequency());

    while (m_IsRunning)
    {
        m_Window->PollEvents(m_IsRunning, m_Minimized);

        if (!m_Minimized)
        {
            uint64_t currentTicks = SDL_GetPerformanceCounter();
            float deltaTime = static_cast<float>((currentTicks - previousTicks) / frequency);
            previousTicks = currentTicks;

            m_Scene->Update(deltaTime);
            m_RenderSystem->Update(deltaTime);

            m_ResourceManager->Clean();
        }
    }
}

void Application::Shutdown()
{
    LogMsg("Application Shutdown.");

    if (m_RenderSystem != nullptr)
    {
        delete m_RenderSystem;
        m_RenderSystem = nullptr;
    }
    if (m_Scene != nullptr)
    {
        delete m_Scene;
        m_Scene = nullptr;
    }
    if (m_ResourceManager != nullptr)
    {
        delete m_ResourceManager;
        m_ResourceManager = nullptr;
    }
    if (m_GraphicsDevice != nullptr)
    {
        delete m_GraphicsDevice;
        m_GraphicsDevice = nullptr;
    }
    if(m_Window != nullptr)
    {
        delete m_Window;
        m_Window = nullptr;
    }
}

} // namespace aero3d