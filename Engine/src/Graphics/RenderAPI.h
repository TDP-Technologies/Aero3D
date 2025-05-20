#ifndef AERO3D_GRAPHICS_RENDERAPI_H_
#define AERO3D_GRAPHICS_RENDERAPI_H_

#include <memory>

#include "Graphics/Buffer.h"
#include "Graphics/ConstantBuffer.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/Texture.h"

namespace aero3d {

class RenderAPI
{
public:
    enum class API
    {
        None,
        Vulkan,
        DirectX12
    };

public:
    virtual ~RenderAPI() = default;

    virtual bool Init() = 0;
    virtual void Shutdown() = 0;

    virtual void RecordCommands() = 0;
    virtual void EndCommands() = 0;

    virtual void SetViewport(int x, int y, int width, int height) = 0;

    virtual void SetClearColor(float r, float g, float b, float a) = 0;
    virtual void Clear() = 0;

    virtual void Draw(std::shared_ptr<VertexBuffer> vb, size_t count) = 0;
    virtual void DrawIndexed(std::shared_ptr<VertexBuffer> vb, std::shared_ptr<IndexBuffer> ib) = 0;

    virtual std::shared_ptr<VertexBuffer> CreateVertexBuffer(void* data, size_t size) = 0;
    virtual std::shared_ptr<IndexBuffer> CreateIndexBuffer(void* data, size_t size, size_t count) = 0;
    virtual std::shared_ptr<ConstantBuffer> CreateConstantBuffer(void* data, size_t size) = 0;

    virtual std::shared_ptr<GraphicsPipeline> CreateGraphicsPipeline(VertexLayout& vertexLayout, 
        std::string& vertexPath, std::string& pixelPath) = 0;
    virtual std::shared_ptr<Texture> CreateTexture(std::string& path) = 0;

    virtual RenderAPI::API GetAPI() = 0;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_RENDERAPI_H_