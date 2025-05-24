#include "IO/NativeVFile.h"

namespace aero3d {

NativeVFile::NativeVFile(void* handle, std::string& virtualPath)
    : m_Handle(handle), m_VirtualPath(virtualPath), m_Opened(false)
{
}

NativeVFile::~NativeVFile()
{
}

void NativeVFile::ReadBytes(void* buffer, size_t size, size_t start)
{
}

std::string NativeVFile::ReadString()
{
}

void NativeVFile::Truncate(size_t pos)
{
}

void NativeVFile::WriteBytes(void* data, size_t size, size_t start)
{
}

void NativeVFile::Load()
{
}

void NativeVFile::Unload()
{
}

bool NativeVFile::IsWritable()
{
}

bool NativeVFile::IsOpened()
{
}

void* NativeVFile::GetData()
{
}

uint64_t NativeVFile::GetLength() const
{
}

const std::string& NativeVFile::GetName() const
{
}

} // namespace aero3d