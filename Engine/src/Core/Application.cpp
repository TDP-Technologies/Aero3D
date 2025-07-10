#include "Application.h"

#include "Utils/Log.h"
#include "IO/VFS.h"
#include "Event/EventBus.h"

#include "Scene/Actor.h"
#include "Scene/Components.h"

#include "Graphics/Vulkan/VulkanGraphicsDevice.h"

namespace aero3d {

bool Application::Init()
{
    LogMsg("Application Initialize.");

    VFS::Mount("", "Sandbox/");

    m_Window = new Window("Aero3D", 800, 600);
    m_GraphicsDevice = new VulkanGraphicsDevice(static_cast<SDL_Window*>(m_Window->GetSDLWindow()));
    m_ResourceManager = new ResourceManager(m_GraphicsDevice, m_GraphicsDevice->GetResourceFactory());
    m_Scene = new Scene();

    EventBus::Subscribe(typeid(WindowResizeEvent), [&](Event& event) 
    {
        WindowResizeEvent& windowResizeEvent = static_cast<WindowResizeEvent&>(event);
        m_GraphicsDevice->GetSwapchain()->Resize();
    });

    m_PerformanceFrequency = static_cast<double>(SDL_GetPerformanceFrequency());
    m_PreviousTicks = SDL_GetPerformanceCounter();

    m_IsRunning = true;

    return true;
}

void Application::Run()
{
    Ref<CommandList> cl = m_GraphicsDevice->CreateCommandList();

    EventBus::Subscribe(typeid(WindowResizeEvent), [&](Event& event) 
    {
        WindowResizeEvent& windowResizeEvent = static_cast<WindowResizeEvent&>(event);
        m_GraphicsDevice->GetSwapchain()->Resize();
    });

    ResourceFactory* rf = m_GraphicsDevice->GetResourceFactory();

    ShaderDesc sd;
    sd.entryPoint = "main";
    sd.path = "res/shaders/vertex";
    sd.stage = STAGE_VERTEX;

    Ref<Shader> vs = rf->CreateShader(sd);

    sd.path = "res/shaders/pixel";
    sd.stage = STAGE_FRAGMENT;
   
    Ref<Shader> fs = rf->CreateShader(sd);

    float vert[] = {
         0.0f, -0.5f, 0.0f,             0.5f, 1.0f,      1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, 0.0f,             1.0f, 0.0f,      0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,             0.0f, 0.0f,      0.0f, 0.0f, 1.0f
    };
    
    unsigned short indices[] = {
        0, 1, 2
    };

    BufferDesc vbd;
    vbd.size = sizeof(vert);
    vbd.usage = USAGE_VERTEX;

    Ref<DeviceBuffer> vdb = rf->CreateBuffer(vbd);
    m_GraphicsDevice->UpdateBuffer(vdb, vert, sizeof(vert));

    BufferDesc ibd;
    ibd.size = sizeof(indices);
    ibd.usage = USAGE_INDEX;

    Ref<DeviceBuffer> idb = rf->CreateBuffer(ibd);
    m_GraphicsDevice->UpdateBuffer(idb, indices, sizeof(indices));

    ResourceLayoutDesc ld;
    ld.bindings = {
        {0, ResourceKind::UNIFORMBUFFER, STAGE_VERTEX},
        {1, ResourceKind::COMBINED_IMAGE_SAMPLER_ARRAY, STAGE_FRAGMENT, 2},
    };

    Ref<ResourceLayout> rl = rf->CreateResourceLayout(ld);

    struct Vertex {
        float inPos[3];
        float inUV[2];
        float inColor[3];
    };

    PipelineDesc pd;
    pd.vertexShader = vs;
    pd.fragmentShader = fs;
    pd.resourceLayout = rl;

    pd.vertexLayout.bindings = {
        { 0, sizeof(Vertex), false }
    };

    pd.vertexLayout.attributes = {
        { 0, 0, VertexFormat::FLOAT3, offsetof(Vertex, inPos) },
        { 1, 0, VertexFormat::FLOAT2, offsetof(Vertex, inUV) },
        { 2, 0, VertexFormat::FLOAT3, offsetof(Vertex, inColor) }
    };

    pd.topology = PrimitiveTopology::TRIANGLELIST;
    pd.cullMode = CullMode::BACK;
    pd.frontFace = FrontFace::CLOCKWISE;
    pd.polygonMode = PolygonMode::FILL;

    pd.depthTest = true;
    pd.depthWrite = true;

    Ref<Pipeline> p = rf->CreatePipeline(pd);

    Ref<TextureView> tv = m_ResourceManager->LoadTexture("res/textures/texture.jpg");
    
    SamplerDesc tsd;
    tsd.filter = SamplerFilter::LINEAR;
    tsd.addressModeU = SamplerAddressMode::REPEAT;

    Ref<Sampler> s = rf->CreateSampler(tsd);

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    UniformBufferObject uboData = {};

    float aspectRatio = 800.0f / 600.0f;

    uboData.model = glm::mat4(1.0f);

    uboData.view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    uboData.proj = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 100.0f);

    BufferDesc ubd;
    ubd.size = sizeof(UniformBufferObject);
    ubd.usage = USAGE_UNIFORM;

    Ref<DeviceBuffer> uniformBuffer = rf->CreateBuffer(ubd);
    m_GraphicsDevice->UpdateBuffer(uniformBuffer, &uboData, sizeof(uboData));

    EventBus::Subscribe(typeid(WindowResizeEvent), [&](Event& event) 
    {
        WindowResizeEvent& windowResizeEvent = static_cast<WindowResizeEvent&>(event);
        
        float aspectRatio = static_cast<float>(windowResizeEvent.GetWidth()) / static_cast<float>(windowResizeEvent.GetHeight());

        uboData.view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        uboData.proj = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 100.0f);
        
        m_GraphicsDevice->UpdateBuffer(uniformBuffer, &uboData, sizeof(uboData));
    });

    std::vector<std::pair<Ref<TextureView>, Ref<Sampler>>> vecPairs = {
        std::make_pair(tv, s),
        std::make_pair(tv, s)
    };

    ResourceSetDesc rsd;
    rsd.layout = rl;
    rsd.resources = { uniformBuffer, vecPairs };

    Ref<ResourceSet> rs = rf->CreateResourceSet(rsd);

    while (m_IsRunning)
    {
        m_Window->PollEvents(m_IsRunning, m_Minimized);

        if (!m_Minimized)
        {
            uint64_t currentTicks = SDL_GetPerformanceCounter();
            float deltaTime = static_cast<float>((currentTicks - m_PreviousTicks) / m_PerformanceFrequency);
            m_PreviousTicks = currentTicks;

            m_Scene->Update(deltaTime);

            cl->Begin();
            cl->SetFramebuffer(m_GraphicsDevice->GetSwapchain()->GetFramebuffer());
            cl->SetPipeline(p);
            cl->SetResourceSet(0, rs);
            cl->SetVertexBuffer(vdb);
            cl->SetIndexBuffer(idb, IndexFormat::UNSIGNED_SHORT);
            cl->DrawIndexed(3);
            cl->End();
            m_GraphicsDevice->SubmitCommands(cl);
            m_GraphicsDevice->Present();

            m_ResourceManager->Clean();
        }
    }
}

void Application::Shutdown()
{
    LogMsg("Application Shutdown.");

    if (m_Scene != nullptr)
    {
        delete m_Scene;
        m_Scene = nullptr;
    }
    if (m_ResourceManager != nullptr)
    {
        delete m_ResourceManager;
        m_ResourceManager = nullptr;
    }
    if (m_GraphicsDevice != nullptr)
    {
        delete m_GraphicsDevice;
        m_GraphicsDevice = nullptr;
    }
    if(m_Window != nullptr)
    {
        delete m_Window;
        m_Window = nullptr;
    }
}

} // namespace aero3d