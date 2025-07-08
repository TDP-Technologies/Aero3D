#include "Resource/ResourceManager.h"

#include "IO/VFS.h"
#include "Utils/ImageLoader.h"

namespace aero3d {

ResourceManager::ResourceManager(GraphicsDevice* graphicsDevice, ResourceFactory* resourceFactory)
{
    m_GraphicsDevice = graphicsDevice;
    m_ResourceFactory = resourceFactory;
}

ResourceManager::~ResourceManager()
{
    Clean();
}

Ref<Texture> ResourceManager::LoadTexture(std::string path)
{
    auto it = m_Textures.find(path);
    if (it != m_Textures.end())
    {
        if (auto existing = it->second.lock())
        {
            return existing;
        }
    }

    ImageData id = ImageLoader::LoadImage("res/textures/texture.jpg");

    TextureDesc td;
    td.width = id.width;
    td.height = id.height;
    td.format = id.format;
    td.usage = TextureUsage::SAMPLED;

    Ref<Texture> texture = m_ResourceFactory->CreateTexture(td);

    m_GraphicsDevice->UpdateTexture(texture, id.pixels.data(), id.pixels.size());

    m_Textures[path] = texture;

    return texture;
}

void ResourceManager::Clean()
{
    for (auto it = m_Textures.begin(); it != m_Textures.end(); )
    {
        if (it->second.expired())
        {
            it = m_Textures.erase(it);
        } 
        else
        {
            ++it;
        }
    }
}

} // namespace aero3d
