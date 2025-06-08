#ifndef AERO3D_CORE_APPLICATION_H_
#define AERO3D_CORE_APPLICATION_H_

#include "Utils/Common.h"

namespace aero3d {

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

};

} // namespace aero3d

#endif // AERO3D_CORE_APPLICATION_H_