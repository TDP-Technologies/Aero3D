#ifndef AERO3D_GRAPHICS_RESOURCES_H_
#define AERO3D_GRAPHICS_RESOURCES_H_

#include <string>
#include <vector>

#include "Utils/Common.h"

namespace aero3d {

class VertexBuffer
{
public:
    ~VertexBuffer() = default;

    virtual void Bind() = 0;

};

enum class IndexBufferType
{
    UNSIGNED_SHORT,
    UNSIGNED_INT
};

class IndexBuffer
{
public:
    ~IndexBuffer() = default;

    virtual void Bind() = 0;

    size_t GetIndexCount() { return m_Count; }

    void SetIndexBufferType(IndexBufferType type) { m_Type = type; }
    IndexBufferType GetIndexBufferType() { return m_Type; }

protected:
    IndexBufferType m_Type = IndexBufferType::UNSIGNED_INT;
    size_t m_Count = 0;

};

class ConstantBuffer
{
public:
    virtual ~ConstantBuffer() = default;

    virtual void Bind(size_t slot) = 0;

    virtual void WriteData(void* data) = 0;

};

class Texture
{
public:
    virtual ~Texture() = default;

    virtual void Bind(unsigned int slot) = 0;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_RESOURCES_H_