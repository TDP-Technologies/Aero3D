#ifndef AERO3D_GRAPHICS_ALLOCATORS_H_
#define AERO3D_GRAPHICS_ALLOCATORS_H_

#include "Graphics/Resources.h"

namespace aero3d {

class ConstantBufferAllocator
{
public:
    virtual ~ConstantBufferAllocator() = default;

    virtual ConstantBuffer Allocate(size_t size) = 0;
    virtual void Free(ConstantBuffer buffer) = 0;

    virtual void NextFrame(size_t index) = 0;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_ALLOCATORS_H_