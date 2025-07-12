#ifndef AERO3D_SYSTEMS_RENDERSYSTEM_H_
#define AERO3D_SYSTEMS_RENDERSYSTEM_H_

#include <glm/glm.hpp>

#include "Graphics/GraphicsDevice.h"
#include "Graphics/ResourceFactory.h"
#include "Scene/Scene.h"

namespace aero3d {

struct SpriteVertex 
{
    glm::vec3 position;
    glm::vec2 uv;
    float texIndex;
};

constexpr uint32_t MAX_TEXTURE_SLOTS = 32;
constexpr uint32_t MAX_QUADS = 1000;
constexpr uint32_t VERTICES_PER_QUAD = 6;
constexpr uint32_t MAX_VERTICES = MAX_QUADS * VERTICES_PER_QUAD;    

class RenderSystem
{
public:
    RenderSystem(GraphicsDevice* graphicsDevice, ResourceFactory* resourceFactory);
    ~RenderSystem();

    void Update(Scene* scene);

    void RenderSprites(Scene* scene);

    void BeginBatch();
    void Flush();

    void DrawQuad(const glm::mat4& transform, Ref<TextureView> texture);

private:
    void Prepare2D();

private:
    GraphicsDevice* m_GraphicsDevice = nullptr;
    ResourceFactory* m_ResourceFactory = nullptr;

    Ref<CommandList> m_CommandList = nullptr;

    Ref<ResourceLayout> m_SpriteResourceLayout = nullptr;
    Ref<Pipeline> m_SpritePipeline = nullptr;
    Ref<DeviceBuffer> m_SpriteVertexBuffer = nullptr;
    Ref<Sampler> m_SpriteTextureSampler = nullptr;

    std::vector<SpriteVertex> m_SpriteVertices;
    std::array<Ref<TextureView>, MAX_TEXTURE_SLOTS> m_TextureSlots;
    uint32_t m_VertexCount = 0;
    uint32_t m_TextureSlotIndex = 0;

};

} // namespace aero3d

#endif // AERO3D_SYSTEMS_RENDERSYSTEM_H_