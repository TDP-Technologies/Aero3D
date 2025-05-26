#include "Graphics/RenderCommand.h"

#include <string.h>

#include "Core/Configuration.h"
#include "Utils/Common.h"
#include "Platform/Vulkan/VulkanAPI.h"

namespace aero3d {

Scope<RenderAPI> RenderCommand::s_API = std::make_unique<VulkanAPI>();

bool RenderCommand::Init(const char* api)
{
    A3D_CHECK_INIT(s_API->Init());

    return true;
}

void RenderCommand::Shutdown()
{
    A3D_SHUTDOWN(s_API);
}

void RenderCommand::RecordCommands()
{
    s_API->RecordCommands();
}

void RenderCommand::EndCommands()
{
    s_API->EndCommands();
}

void RenderCommand::ResizeBuffers()
{
    s_API->ResizeBuffers();
}

void RenderCommand::SetViewport(int x, int y, int width, int height)
{
    s_API->SetViewport(x, y, width, height);
}

void RenderCommand::SetClearColor(float r, float g, float b, float a)
{
    s_API->SetClearColor(r, g, b, a);
}

void RenderCommand::Clear()
{
    s_API->Clear();
}

void RenderCommand::Draw(Ref<VertexBuffer> vb, size_t count)
{
    s_API->Draw(vb, count);
}

void RenderCommand::DrawIndexed(Ref<VertexBuffer> vb, Ref<IndexBuffer> ib)
{
    s_API->DrawIndexed(vb, ib);
}

Ref<VertexBuffer> RenderCommand::CreateVertexBuffer(void* data, size_t size)
{
    return s_API->CreateVertexBuffer(data, size);
}

Ref<IndexBuffer> RenderCommand::CreateIndexBuffer(void* data, size_t size, size_t count)
{
    return s_API->CreateIndexBuffer(data, size, count);
}

Ref<ConstantBuffer> RenderCommand::CreateConstantBuffer(void* data, size_t size)
{
    return s_API->CreateConstantBuffer(data, size);
}

Ref<GraphicsPipeline> RenderCommand::CreateGraphicsPipeline(VertexLayout& vertexLayout,
    std::string vertexPath, std::string pixelPath)
{
    return s_API->CreateGraphicsPipeline(vertexLayout, vertexPath, pixelPath);
}

Ref<Texture> RenderCommand::CreateTexture(std::string path)
{
    return s_API->CreateTexture(path);
}

RenderAPI::API RenderCommand::GetAPI()
{
    return s_API->GetAPI();
}

} // namespace aero3d