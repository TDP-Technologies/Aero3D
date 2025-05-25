#ifndef AERO3D_IO_NATIVEVFILE_H_
#define AERO3D_IO_NATIVEVFILE_H_

#include <string>

#include "IO/VFile.h"

#ifdef _WIN32
    using FileHandle = void*;
    #define NullHandle = nullptr;
#else
    using FileHandle = int;
    #define NullHandle -1;
#endif

namespace aero3d {

class NativeVFile : public VFile
{
public:
    NativeVFile(FileHandle handle, std::string& virtualPath);
    ~NativeVFile();

    virtual void ReadBytes(void* buffer, size_t size, size_t start = 0) override;
    virtual std::string ReadString() override;

    virtual void Truncate(size_t pos = 0) override;
    virtual void WriteBytes(void* data, size_t size, size_t start = 0) override;

    virtual void Load() override;
    virtual void Unload() override;

    virtual bool IsWritable() override;
    virtual bool IsOpened() override;

    virtual void* GetData() override;

    virtual uint64_t GetLength() const override;
    virtual const std::string& GetName() const override;

private:
    uint64_t m_Length = 0;
    std::string m_VirtualPath = "";
    FileHandle m_Handle = NullHandle;
    void* m_Data = nullptr;
    bool m_Opened = false;

};

} // namespace aero3d

#endif // AERO3D_IO_NATIVEVFILE_H_