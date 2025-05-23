#ifndef AERO3D_GRAPHICS_RENDERCOMMAND_H_
#define AERO3D_GRAPHICS_RENDERCOMMAND_H_

#include <memory>

#include "Graphics/Buffer.h"
#include "Graphics/ConstantBuffer.h"
#include "Graphics/RenderAPI.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"

namespace aero3d {

class RenderCommand
{
public:
    static bool Init(const char* api);
    static void Shutdown();

    static void SetViewport(int x, int y, int width, int height);

    static void SetClearColor(float r, float g, float b, float a);
    static void Clear();

    static void Draw(std::shared_ptr<VertexBuffer> vb, size_t count);
    static void DrawIndexed(std::shared_ptr<VertexBuffer> vb, std::shared_ptr<IndexBuffer> ib);

    static std::shared_ptr<VertexBuffer> CreateVertexBuffer(BufferLayout& layout, const void* data, size_t size);
    static std::shared_ptr<IndexBuffer> CreateIndexBuffer(const void* data, size_t size, size_t count);
    static std::shared_ptr<ConstantBuffer> CreateConstantBuffer(const void* data, size_t size);

    static std::shared_ptr<Shader> CreateShader(std::string vertexPath, std::string pixelPath);
    static std::shared_ptr<Texture> CreateTexture(std::string path);

    static RenderAPI::API GetAPI();

private:
    static std::unique_ptr<RenderAPI> s_API;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_RENDERCOMMAND_H_