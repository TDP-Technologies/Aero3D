#ifndef AERO3D_GRAPHICS_COMMANDBUFFER_H_
#define AERO3D_GRAPHICS_COMMANDBUFFER_H_

#include "Utils/Common.h"
#include "Graphics/Resources.h"

namespace aero3d {

class CommandBuffer
{
public:
    virtual ~CommandBuffer() = default;

    virtual void Record() = 0;
    virtual void End() = 0;

    virtual void Draw(Ref<Buffer> vb) = 0;
    virtual void DrawIndexed(Ref<Buffer> vb, Ref<Buffer> ib) = 0;

    virtual void BindPipeline(Ref<Pipeline> pipeline) = 0;

    virtual void Execute() = 0;

    virtual Ref<Buffer> CreateBuffer(Buffer::Description desc) = 0;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_COMMANDBUFFER_H_