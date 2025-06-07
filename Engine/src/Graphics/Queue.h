#ifndef AERO3D_GRAPHICS_QUEUE_H_
#define AERO3D_GRAPHICS_QUEUE_H_

#include "Graphics/CommandBuffer.h"

namespace aero3d {

class Queue
{
public:
    virtual ~Queue() = default;

    virtual void Execute(Ref<CommandBuffer> buffer) = 0;

};
    
} // namespace aero3d

#endif // AERO3D_GRAPHICS_QUEUE_H_