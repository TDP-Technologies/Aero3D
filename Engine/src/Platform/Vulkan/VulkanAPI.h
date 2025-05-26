#ifndef AERO3D_PLATFORM_VULKAN_VULKANAPI_H_
#define AERO3D_PLATFORM_VULKAN_VULKANAPI_H_

#include "Graphics/RenderAPI.h"

namespace aero3d {

class VulkanAPI : public RenderAPI
{
public:
    VulkanAPI();
    ~VulkanAPI();

    virtual bool Init() override;
    virtual void Shutdown() override;

    virtual void ResizeBuffers(int width, int height) override;

    virtual void RecordCommands() override;
    virtual void EndCommands() override;

    virtual void SetClearColor(float r, float g, float b, float a) override;
    virtual void Clear() override;

    virtual void Draw(Ref<VertexBuffer> vb, size_t count) override;
    virtual void DrawIndexed(Ref<VertexBuffer> vb, Ref<IndexBuffer> ib) override;

    virtual Ref<VertexBuffer> CreateVertexBuffer(void* data, size_t size) override;
    virtual Ref<IndexBuffer> CreateIndexBuffer(void* data, size_t size, size_t count) override;
    virtual Ref<ConstantBuffer> CreateConstantBuffer(void* data, size_t size) override;

    virtual Ref<GraphicsPipeline> CreateGraphicsPipeline(VertexLayout& vertexLayout,
        std::string& vertexPath, std::string& pixelPath) override;
    virtual Ref<Texture> CreateTexture(std::string& path) override;

    virtual RenderAPI::API GetAPI() override;

private:


};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_VULKANAPI_H_