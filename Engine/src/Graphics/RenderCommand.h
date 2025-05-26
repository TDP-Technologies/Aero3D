#ifndef AERO3D_GRAPHICS_RENDERCOMMAND_H_
#define AERO3D_GRAPHICS_RENDERCOMMAND_H_

#include <memory>

#include "Utils/Common.h"
#include "Graphics/Buffer.h"
#include "Graphics/RenderAPI.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/Texture.h"

namespace aero3d {

class RenderCommand
{
public:
    static bool Init(const char* api);
    static void Shutdown();

    static void RecordCommands();
    static void EndCommands();

    static void ResizeBuffers(int width, int height);

    static void SetClearColor(float r, float g, float b, float a);
    static void Clear();

    static void Draw(Ref<VertexBuffer> vb, size_t count);
    static void DrawIndexed(Ref<VertexBuffer> vb, Ref<IndexBuffer> ib);

    static Ref<VertexBuffer> CreateVertexBuffer(void* data, size_t size);
    static Ref<IndexBuffer> CreateIndexBuffer(void* data, size_t size, size_t count);
    static Ref<ConstantBuffer> CreateConstantBuffer(void* data, size_t size);

    static Ref<GraphicsPipeline> CreateGraphicsPipeline(VertexLayout& vertexLayout,
        std::string vertexPath, std::string pixelPath);
    static Ref<Texture> CreateTexture(std::string path);

    static RenderAPI::API GetAPI();

private:
    static Scope<RenderAPI> s_API;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_RENDERCOMMAND_H_