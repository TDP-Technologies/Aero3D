#ifndef AERO3D_RESOURCE_RESOURCEMANAGER_H_
#define AERO3D_RESOURCE_RESOURCEMANAGER_H_

#include <unordered_map>

#include "Graphics/GraphicsDevice.h"
#include "Graphics/ResourceFactory.h"

namespace aero3d {

class ResourceManager
{
public:
    ResourceManager(GraphicsDevice* graphicsDevice, ResourceFactory* resourceFactory);
    ~ResourceManager();

    Ref<Texture> LoadTexture(std::string path);

    void Clean();

private:
    GraphicsDevice* m_GraphicsDevice = nullptr;
    ResourceFactory* m_ResourceFactory = nullptr;
    std::unordered_map<std::string, std::weak_ptr<Texture>> m_Textures;

};

} // namespace aero3d

#endif // AERO3D_RESOURCE_RESOURCEMANAGER_H_