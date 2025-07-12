#include "Application.h"

#include <SDL3/SDL.h>

#include "Utils/Log.h"
#include "IO/VFS.h"
#include "Event/EventBus.h"

#include "Scene/Actor.h"
#include "Scene/Components.h"

#include "Systems/RenderSystem.h"
#include "Graphics/Vulkan/VulkanGraphicsDevice.h"

namespace aero3d {

bool Application::Init()
{
    LogMsg("Application Initialize.");

    VFS::Mount("", "Sandbox/");

    m_Window = new Window("Aero3D", 800, 600);

    RenderSurfaceCreateInfo renderSurfaceInfo;
    renderSurfaceInfo.type = RenderSurfaceCreateInfo::WindowType::SDL;
    renderSurfaceInfo.sdlWindow = m_Window->GetSDLWindow();

    m_GraphicsDevice = new VulkanGraphicsDevice(renderSurfaceInfo);
    m_ResourceManager = new ResourceManager(m_GraphicsDevice, m_GraphicsDevice->GetResourceFactory());
    m_Scene = new Scene();
    m_RenderSystem = new RenderSystem(m_GraphicsDevice, m_GraphicsDevice->GetResourceFactory());

    EventBus::Subscribe(typeid(WindowResizeEvent), [&](Event& event) 
    {
        WindowResizeEvent& windowResizeEvent = static_cast<WindowResizeEvent&>(event);
        m_GraphicsDevice->GetSwapchain()->Resize();
    });

    m_PerformanceFrequency = static_cast<double>(SDL_GetPerformanceFrequency());
    m_PreviousTicks = SDL_GetPerformanceCounter();

    m_IsRunning = true;

    return true;
}

void Application::Run()
{
    auto actor = std::make_unique<Actor>();

    auto sprite = std::make_unique<SpriteComponent>();
    sprite->SetTexture(m_ResourceManager->LoadTexture("res/textures/texture.jpg"));
    sprite->SetLocalTransform(glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)));

    actor->SetRootComponent(sprite.get());
    actor->AddComponent(std::move(sprite));

    m_Scene->AddActor(std::move(actor));

    while (m_IsRunning)
    {
        m_Window->PollEvents(m_IsRunning, m_Minimized);

        if (!m_Minimized)
        {
            uint64_t currentTicks = SDL_GetPerformanceCounter();
            float deltaTime = static_cast<float>((currentTicks - m_PreviousTicks) / m_PerformanceFrequency);
            m_PreviousTicks = currentTicks;

            m_Scene->Update(deltaTime);
            m_RenderSystem->Update(m_Scene);

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