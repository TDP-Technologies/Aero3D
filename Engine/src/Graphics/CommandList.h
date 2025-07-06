#ifndef AERO3D_GRAPHICS_COMMANDLIST_H_
#define AERO3D_GRAPHICS_COMMANDLIST_H_

#include <cstdint>

#include "Graphics/Resources.h"
#include "Utils/Common.h"

namespace aero3d {

class CommandList 
{
public:
    virtual ~CommandList() = default;

    virtual void Begin() = 0;
    virtual void End() = 0;

    virtual void SetFramebuffer(Ref<Framebuffer> framebuffer) = 0;
    virtual void SetPipeline(Ref<Pipeline> pipeline) = 0;
    virtual void SetVertexBuffer(Ref<DeviceBuffer> buffer, uint32_t offset = 0) = 0;
    virtual void SetIndexBuffer(Ref<DeviceBuffer> buffer, IndexFormat format, uint32_t offset = 0) = 0;
    virtual void SetResourceSet(uint32_t slot, Ref<ResourceSet> resourceSet) = 0;

    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1) = 0;
    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1) = 0;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_COMMANDLIST_H_