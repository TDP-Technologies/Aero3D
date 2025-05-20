#ifndef AERO3D_GRAPHICS_BUFFER_H_
#define AERO3D_GRAPHICS_BUFFER_H_

#include <string>
#include <vector>

#include "Utils/Common.h"

namespace aero3d {

class A3D_API VertexBuffer
{
public:
    ~VertexBuffer() = default;

    virtual void Bind() = 0;
    virtual void Unbind() = 0;

    virtual void SetData(const void* data, size_t size) = 0;

};

enum class IndexBufferType
{
    UNSIGNED_BYTE,
    UNSIGNED_SHORT,
    UNSIGNED_INT
};

class A3D_API IndexBuffer
{
public:
    IndexBuffer(IndexBufferType type = IndexBufferType::UNSIGNED_INT)
        : m_Type(type) {}
    ~IndexBuffer() = default;

    virtual void Bind() = 0;
    virtual void Unbind() = 0;

    virtual size_t GetIndexCount() = 0;

    void SetIndexBufferType(IndexBufferType type) { m_Type = type; }
    IndexBufferType GetIndexBufferType() { return m_Type; }

protected:
    IndexBufferType m_Type;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_BUFFER_H_