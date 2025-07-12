#include "Systems/RenderSystem.h"

#include "Scene/Components.h"

namespace aero3d {

RenderSystem::RenderSystem(GraphicsDevice* graphicsDevice, ResourceFactory* resourceFactory)
{
    m_GraphicsDevice = graphicsDevice;
    m_ResourceFactory = resourceFactory;

    m_CommandList = graphicsDevice->CreateCommandList();

    Prepare2D();
}

RenderSystem::~RenderSystem()
{

}

void RenderSystem::Update(Scene* scene)
{
    m_CommandList->Begin();
    m_CommandList->SetFramebuffer(m_GraphicsDevice->GetSwapchain()->GetFramebuffer());
    m_CommandList->ClearRenderTargets(0.0f, 0.0f, 0.0f, 1.0f);
    m_CommandList->ClearDepthStencil();
    m_CommandList->End();
    m_GraphicsDevice->SubmitCommands(m_CommandList);
    RenderSprites(scene);
    m_GraphicsDevice->Present();
}

void RenderSystem::RenderSprites(Scene* scene)
{
    BeginBatch();
    for (auto& sprite : scene->GetAllComponentsOfType<SpriteComponent>())
    {
        DrawQuad(sprite->GetWorldTransform(), sprite->GetTexture());
    }
    Flush();
}

void RenderSystem::BeginBatch()
{
    m_VertexCount = 0;
    m_TextureSlotIndex = 0;
    m_SpriteVertices.clear();
}

void RenderSystem::Flush()
{
    if (m_VertexCount == 0)
        return;

    m_GraphicsDevice->UpdateBuffer(
        m_SpriteVertexBuffer,
        m_SpriteVertices.data(),
        m_VertexCount * sizeof(SpriteVertex)
    );

    std::vector<Ref<TextureView>> textures;

    Ref<TextureView> lastValidTexture = nullptr;

    for (const auto& tex : m_TextureSlots)
    {
        if (tex)
            lastValidTexture = tex;
    }

    for (const auto& tex : m_TextureSlots)
    {
        if (tex)
            textures.push_back(tex);
        else
            textures.push_back(lastValidTexture);
    }

    ResourceSetDesc setDesc;
    setDesc.layout = m_SpriteResourceLayout;
    setDesc.resources = { m_SpriteTextureSampler, textures };

    Ref<ResourceSet> resourceSet = m_ResourceFactory->CreateResourceSet(setDesc);

    m_CommandList->Begin();
    m_CommandList->SetFramebuffer(m_GraphicsDevice->GetSwapchain()->GetFramebuffer());
    m_CommandList->SetPipeline(m_SpritePipeline);
    m_CommandList->SetResourceSet(0, resourceSet);
    m_CommandList->SetVertexBuffer(m_SpriteVertexBuffer);
    m_CommandList->Draw(m_VertexCount);
    m_CommandList->End();
    m_GraphicsDevice->SubmitCommands(m_CommandList);
}

void RenderSystem::DrawQuad(const glm::mat4& transform, Ref<TextureView> texture)
{
    if (m_VertexCount + 6 > MAX_VERTICES)
    {
        Flush();
        BeginBatch();
    }

    float textureIndex = -1.0f;
    for (uint32_t i = 0; i < m_TextureSlotIndex; ++i)
    {
        if (m_TextureSlots[i] == texture)
        {
            textureIndex = static_cast<float>(i);
            break;
        }
    }

    if (textureIndex == -1.0f)
    {
        if (m_TextureSlotIndex >= MAX_TEXTURE_SLOTS)
        {
            Flush();
            BeginBatch();
        }

        textureIndex = static_cast<float>(m_TextureSlotIndex);
        m_TextureSlots[m_TextureSlotIndex++] = texture;
    }

    glm::vec4 quadVerts[4] = 
    {
        { -0.5f, -0.5f, 0.0f, 1.0f },
        {  0.5f, -0.5f, 0.0f, 1.0f },
        {  0.5f,  0.5f, 0.0f, 1.0f },
        { -0.5f,  0.5f, 0.0f, 1.0f },
    };

    glm::vec2 uvs[4] = 
    {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
    };

    uint32_t order[6] = { 0, 1, 2, 2, 3, 0 };

    for (uint32_t i = 0; i < 6; ++i)
    {
        uint32_t v = order[i];
        glm::vec4 worldPos = transform * quadVerts[v];

        SpriteVertex vertex;
        vertex.position = glm::vec3(worldPos);
        vertex.uv = uvs[v];
        vertex.texIndex = textureIndex;

        m_SpriteVertices.push_back(vertex);
    }

    m_VertexCount += 6;
}

void RenderSystem::Prepare2D()
{
    ShaderDesc shaderDescription;
    shaderDescription.entryPoint = "main";
    shaderDescription.path = "res/shaders/vertex";
    shaderDescription.stage = STAGE_VERTEX;

    Ref<Shader> vertexShader = m_ResourceFactory->CreateShader(shaderDescription);

    shaderDescription.path = "res/shaders/pixel";
    shaderDescription.stage = STAGE_FRAGMENT;

    Ref<Shader> fragmentShader = m_ResourceFactory->CreateShader(shaderDescription);

    ResourceLayoutDesc layoutDescription;
    layoutDescription.bindings = 
    {
        {0, ResourceKind::Sampler, STAGE_FRAGMENT},
        {1, ResourceKind::TextureReadOnlyArray, STAGE_FRAGMENT, MAX_TEXTURE_SLOTS},
    };

    m_SpriteResourceLayout = m_ResourceFactory->CreateResourceLayout(layoutDescription);

    PipelineDesc pipelineDescription;
    pipelineDescription.vertexShader = vertexShader;
    pipelineDescription.fragmentShader = fragmentShader;
    pipelineDescription.resourceLayout = m_SpriteResourceLayout;

    pipelineDescription.vertexLayout.bindings = 
    {
        { 0, sizeof(SpriteVertex), false }
    };

    pipelineDescription.vertexLayout.attributes = 
    {
        { 0, 0, VertexFormat::Float3, offsetof(SpriteVertex, position) },
        { 1, 0, VertexFormat::Float2, offsetof(SpriteVertex, uv) },
        { 2, 0, VertexFormat::Float, offsetof(SpriteVertex, texIndex) }
    };

    pipelineDescription.topology = PrimitiveTopology::TriangleList;
    pipelineDescription.cullMode = CullMode::Back;
    pipelineDescription.frontFace = FrontFace::ClockWise;
    pipelineDescription.polygonMode = PolygonMode::Fill;

    pipelineDescription.depthTest = true;
    pipelineDescription.depthWrite = true;

    m_SpritePipeline = m_ResourceFactory->CreatePipeline(pipelineDescription);

    BufferDesc bufferDesc;
    bufferDesc.usage = USAGE_VERTEX;
    bufferDesc.size = MAX_VERTICES * sizeof(SpriteVertex);

    m_SpriteVertexBuffer = m_ResourceFactory->CreateBuffer(bufferDesc);
    m_SpriteVertices.reserve(MAX_VERTICES);

    SamplerDesc textureSamplerDescription;
    textureSamplerDescription.filter = SamplerFilter::Linear;
    textureSamplerDescription.addressModeU = SamplerAddressMode::Repeat;

    m_SpriteTextureSampler = m_ResourceFactory->CreateSampler(textureSamplerDescription);
}

} // namespace aero3d