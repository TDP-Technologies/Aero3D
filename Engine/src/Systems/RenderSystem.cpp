#include "Systems/RenderSystem.h"

namespace aero3d {

RenderSystem::RenderSystem(GraphicsDevice* graphicsDevice, ResourceFactory* resourceFactory)
{
    m_GraphicsDevice = graphicsDevice;
    m_ResourceFactory = resourceFactory;

    m_Swapchain = graphicsDevice->GetSwapchain();
    m_CommandList = graphicsDevice->CreateCommandList();

    Prepare2D();
}

RenderSystem::~RenderSystem()
{

}

void RenderSystem::Update(float deltaTime)
{
    m_CommandList->Begin();
    m_CommandList->SetFramebuffer(m_Swapchain->GetFramebuffer());
    m_CommandList->End();
    m_GraphicsDevice->SubmitCommands(m_CommandList);
    m_GraphicsDevice->Present();
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
    layoutDescription.bindings = {
        {0, ResourceKind::UNIFORMBUFFER, STAGE_VERTEX},
        {1, ResourceKind::COMBINED_IMAGE_SAMPLER_ARRAY, STAGE_FRAGMENT, 2},
    };

    m_SpriteResourceLayout = m_ResourceFactory->CreateResourceLayout(layoutDescription);

    struct Vertex 
    {
        float inPos[3];
        float inUV[2];
        float inColor[3];
    };

    PipelineDesc pipelineDescription;
    pipelineDescription.vertexShader = vertexShader;
    pipelineDescription.fragmentShader = fragmentShader;
    pipelineDescription.resourceLayout = m_SpriteResourceLayout;

    pipelineDescription.vertexLayout.bindings = {
        { 0, sizeof(Vertex), false }
    };

    pipelineDescription.vertexLayout.attributes = {
        { 0, 0, VertexFormat::FLOAT3, offsetof(Vertex, inPos) },
        { 1, 0, VertexFormat::FLOAT2, offsetof(Vertex, inUV) },
        { 2, 0, VertexFormat::FLOAT3, offsetof(Vertex, inColor) }
    };

    pipelineDescription.topology = PrimitiveTopology::TRIANGLELIST;
    pipelineDescription.cullMode = CullMode::BACK;
    pipelineDescription.frontFace = FrontFace::CLOCKWISE;
    pipelineDescription.polygonMode = PolygonMode::FILL;

    pipelineDescription.depthTest = true;
    pipelineDescription.depthWrite = true;

    m_SpritePipeline = m_ResourceFactory->CreatePipeline(pipelineDescription);
}

} // namespace aero3d
