#ifndef AERO3D_CORE_APPLICATION_H_
#define AERO3D_CORE_APPLICATION_H_

#include "Core/Window.h"
#include "Graphics/GraphicsDevice.h"

namespace aero3d {

class Application
{
private:
    Application() = default;

    static Application s_Application;

public:
    ~Application() = default;

    bool Init();
    void Run();
    void Shutdown();

    static Application& Get() { return s_Application; }
    static Window* GetWindow() { return s_Application.m_Window; }
    static GraphicsDevice* GetGraphicsDevice() { return s_Application.m_GraphicsDevice; }

private:
    bool m_IsRunning = false;
    bool m_Minimized = false;

    Window* m_Window = nullptr;
    GraphicsDevice* m_GraphicsDevice = nullptr;

};

} // namespace aero3d

#endif // AERO3D_CORE_APPLICATION_H_