#include "Core/Application.h"

int main()
{
    aero3d::Application app = aero3d::Application::Get();
    if (app.Init())
    {
        app.Run();
    }

    app.Shutdown();
    
    return 0;
}