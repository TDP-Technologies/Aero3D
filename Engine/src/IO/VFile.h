#ifndef AERO3D_IO_VFILE_H_
#define AERO3D_IO_VFILE_H_

#include <string>
#include <cstdint>

namespace aero3d {

class VFile
{
public:
    virtual ~VFile() = default;

    virtual void ReadBytes(void* buffer, size_t size, size_t start = 0) = 0;
    virtual std::string ReadString() = 0;

    virtual void Truncate(size_t pos = 0) = 0;
    virtual void WriteBytes(void* data, size_t size, size_t start = 0) = 0;

    virtual void Load() = 0;
    virtual void Unload() = 0;

    virtual bool IsWritable() = 0;
    virtual bool IsOpened() = 0;

    virtual void* GetData() = 0;

    virtual uint64_t GetLength() const = 0;
    virtual const std::string& GetName() const = 0;

};

} // namespace aero3d

#endif // AERO3D_IO_VFILE_H_