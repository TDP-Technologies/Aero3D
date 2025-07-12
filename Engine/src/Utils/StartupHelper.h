#ifndef AERO3D_UTILS_STARTUPHELPER_H_
#define AERO3D_UTILS_STARTUPHELPER_H_

#include "Core/Window.h"
#include "Graphics/GraphicsDevice.h"

namespace aero3d {

class StartupHelper
{
public:
    static GraphicsDevice* CreateGraphicsDevice(RenderingAPI api, RenderSurfaceCreateInfo surfaceInfo);
    static Window* CreateWindow(WindowInfo info);

};

} // namespace aero3d

#endif // AERO3D_UTILS_STARTUPHELPER_H_