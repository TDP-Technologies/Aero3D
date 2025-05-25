#include "Platform/Vulkan/VulkanAPI.h"

#include "Platform/Vulkan/Internal/VulkanCore.h"
#include "Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace aero3d {

VulkanAPI::VulkanAPI()
{
}

VulkanAPI::~VulkanAPI()
{
}

bool VulkanAPI::Init()
{
    return true;
}

void VulkanAPI::Shutdown()
{

}

void VulkanAPI::RecordCommands()
{
    g_VulkanCore->RecordCommands();
}

void VulkanAPI::EndCommands()
{
    g_VulkanCore->EndCommands();
}

void VulkanAPI::SetViewport(int x, int y, int width, int height)
{
    g_VulkanCore->SetViewport(x, y, width, height);
}

void VulkanAPI::SetClearColor(float r, float g, float b, float a)
{
    g_VulkanCore->SetClearColor(r, g, b, a);
}

void VulkanAPI::Clear()
{
    g_VulkanCore->Clear();
}

void VulkanAPI::Draw(Ref<VertexBuffer> vb, size_t count)
{
    vb->Bind();
    g_VulkanCore->Draw(count);
}

void VulkanAPI::DrawIndexed(Ref<VertexBuffer> vb, Ref<IndexBuffer> ib)
{
    vb->Bind();
    ib->Bind();
    g_VulkanCore->DrawIndexed(ib->GetIndexCount());
}

Ref<VertexBuffer> VulkanAPI::CreateVertexBuffer(void* data, size_t size)
{
    return std::make_shared<VulkanVertexBuffer>(data, size);
}

Ref<IndexBuffer> VulkanAPI::CreateIndexBuffer(void* data, size_t size, size_t count)
{
    return std::make_shared<VulkanIndexBuffer>(data, size, count);
}

Ref<ConstantBuffer> VulkanAPI::CreateConstantBuffer(void* data, size_t size)
{
    return nullptr;
}

Ref<GraphicsPipeline> VulkanAPI::CreateGraphicsPipeline(VertexLayout& vertexLayout, std::string& vertexPath, std::string& pixelPath)
{
    return std::make_shared<VulkanGraphicsPipeline>(vertexLayout, vertexPath, pixelPath);
}

Ref<Texture> VulkanAPI::CreateTexture(std::string& path)
{
    return nullptr;
}

RenderAPI::API VulkanAPI::GetAPI()
{
    return RenderAPI::API::Vulkan;
}

} // namespace aero3d