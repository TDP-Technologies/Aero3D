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

void VulkanAPI::Draw(std::shared_ptr<VertexBuffer> vb, size_t count)
{
    vb->Bind();
    g_VulkanCore->Draw(count);
}

void VulkanAPI::DrawIndexed(std::shared_ptr<VertexBuffer> vb, std::shared_ptr<IndexBuffer> ib)
{

}

std::shared_ptr<VertexBuffer> VulkanAPI::CreateVertexBuffer(void* data, size_t size)
{
    return std::make_shared<VulkanVertexBuffer>(data, size);
}

std::shared_ptr<IndexBuffer> VulkanAPI::CreateIndexBuffer(void* data, size_t size, size_t count)
{
    return nullptr;
}

std::shared_ptr<ConstantBuffer> VulkanAPI::CreateConstantBuffer(void* data, size_t size)
{
    return nullptr;
}

std::shared_ptr<GraphicsPipeline> VulkanAPI::CreateGraphicsPipeline(VertexLayout& vertexLayout, std::string& vertexPath, std::string& pixelPath)
{
    return std::make_shared<VulkanGraphicsPipeline>(vertexLayout, vertexPath, pixelPath);
}

std::shared_ptr<Texture> VulkanAPI::CreateTexture(std::string& path)
{
    return nullptr;
}

RenderAPI::API VulkanAPI::GetAPI()
{
    return RenderAPI::API::Vulkan;
}

} // namespace aero3d