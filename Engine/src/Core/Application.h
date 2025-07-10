#ifndef AERO3D_CORE_APPLICATION_H_
#define AERO3D_CORE_APPLICATION_H_

#include "Core/Window.h"
#include "Scene/Scene.h"
#include "Graphics/GraphicsDevice.h"
#include "Resource/ResourceManager.h"

namespace aero3d {

class RenderSystem;

class Application
{
public:
    Application() = default;
    ~Application() = default;

    bool Init();
    void Run();
    void Shutdown();

private:
    bool m_IsRunning = false;
    bool m_Minimized = false;

    uint64_t m_PreviousTicks = 0;
    double m_PerformanceFrequency = 0.0;

    Window* m_Window = nullptr;
    GraphicsDevice* m_GraphicsDevice = nullptr;
    ResourceManager* m_ResourceManager = nullptr;
    Scene* m_Scene = nullptr;
    RenderSystem* m_RenderSystem;
    
};

} // namespace aero3d

#endif // AERO3D_CORE_APPLICATION_H_