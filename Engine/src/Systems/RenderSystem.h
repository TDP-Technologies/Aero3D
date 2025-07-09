#ifndef AERO3D_SYSTEMS_RENDERSYSTEM_H_
#define AERO3D_SYSTEMS_RENDERSYSTEM_H_

#include <glm/glm.hpp>

#include "Graphics/GraphicsDevice.h"
#include "Graphics/Resources.h"

namespace aero3d {

class RenderSystem
{
public:
    RenderSystem(GraphicsDevice* graphicsDevice);
    ~RenderSystem();


private:
    GraphicsDevice* m_GraphicsDevice = nullptr;


};

} // namespace aero3d

#endif // AERO3D_SYSTEMS_RENDERSYSTEM_H_