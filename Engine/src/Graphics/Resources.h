#ifndef AERO3D_GRAPHICS_RESOURCES_H_
#define AERO3D_GRAPHICS_RESOURCES_H_

#include <cstdint>
#include <vector>
#include <string>

namespace aero3d {

class Pipeline
{
public:
enum class AttributeType
{
    UNDEFINED,
    FLOAT, FLOAT2, FLOAT3, FLOAT4, INT, INT2, INT3, INT4,
    BOOL, MAT2, MAT3, MAT4
};

static int AttributeTypeSize(AttributeType type)
{
    switch (type)
    {
    case AttributeType::FLOAT: return 4;
    case AttributeType::FLOAT2: return 8;
    case AttributeType::FLOAT3: return 12;
    case AttributeType::FLOAT4: return 16;
    case AttributeType::INT: return 4;
    case AttributeType::INT2: return 8;
    case AttributeType::INT3: return 12;
    case AttributeType::INT4: return 16;
    case AttributeType::BOOL: return 1;
    case AttributeType::MAT2: return 16;
    case AttributeType::MAT3: return 36;
    case AttributeType::MAT4: return 64;
    default: return 0;
    }
}

struct Attribute
{
    const char* Name = nullptr;
    int Size = 0;
    int Offset = 0;
    AttributeType Type = AttributeType::UNDEFINED;

    Attribute(const char* name, AttributeType type)
        : Name(name), Type(type), Size(AttributeTypeSize(type)) {
    }

    uint32_t GetComponentCount() const
    {
        switch (Type)
        {
        case AttributeType::FLOAT: return 1;
        case AttributeType::FLOAT2: return 2;
        case AttributeType::FLOAT3: return 3;
        case AttributeType::FLOAT4: return 4;
        case AttributeType::INT: return 1;
        case AttributeType::INT2: return 2;
        case AttributeType::INT3: return 3;
        case AttributeType::INT4: return 4;
        case AttributeType::BOOL: return 1;
        case AttributeType::MAT2: return 4;
        case AttributeType::MAT3: return 9;
        case AttributeType::MAT4: return 16;
        default: return 0;
        }
    }

};

class InputAttributeDescription
{
public:
    InputAttributeDescription(std::vector<Attribute>&& elements)
        : m_Attributes(std::move(elements))
    {
        for (Attribute& element : m_Attributes)
        {
            element.Offset = m_Stride;
            m_Stride += element.Size;
        }
    }

    const std::vector<Attribute>& GetAttributes() const { return m_Attributes; };
    int GetStride() const { return m_Stride; }

private:
    int m_Stride = 0;
    std::vector<Attribute> m_Attributes {};

};

enum class ShaderType
{
    VERTEX, PIXEL
};

struct Shader
{
    ShaderType Type;
    std::string Path;
};

struct Description
{
    InputAttributeDescription AttribDesc;
    Shader VertexShader;
    Shader PixelShader;
};

public:
    virtual ~Pipeline() = default;

};
    
class Buffer
{
public:
    enum class BufferType
    {
        VERTEX, INDEX
    };

    enum class IndexType
    {
        UNDEFINED, UNSIGNED_SHORT, UNSIGNED_INT
    };

    struct Description
    {
        BufferType Usage;
        IndexType IndexSize;
        uint32_t Size;
        void* Data;
    };

public:
    virtual ~Buffer() = default;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_RESOURCES_H_