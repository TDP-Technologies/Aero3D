#ifndef AERO3D_SYSTEMS_RENDERSYSTEM_H_
#define AERO3D_SYSTEMS_RENDERSYSTEM_H_

#include "Graphics/GraphicsDevice.h"
#include "Graphics/Resources.h"

namespace aero3d {

class RenderSystem
{
public:
    RenderSystem(GraphicsDevice* graphicsDevice, ResourceFactory* resourceFactory);
    ~RenderSystem();

    void Update(float deltaTime);

private:
    void Prepare2D();

private:
    GraphicsDevice* m_GraphicsDevice = nullptr;
    ResourceFactory* m_ResourceFactory = nullptr;

    Swapchain* m_Swapchain = nullptr;

    Ref<CommandList> m_CommandList = nullptr;

    Ref<ResourceLayout> m_SpriteResourceLayout = nullptr;
    Ref<Pipeline> m_SpritePipeline = nullptr;


};

} // namespace aero3d

#endif // AERO3D_SYSTEMS_RENDERSYSTEM_H_