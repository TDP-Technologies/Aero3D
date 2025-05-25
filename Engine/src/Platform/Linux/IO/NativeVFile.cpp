#include "IO/NativeVFile.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cstring>
#include <limits>
#include <cstddef>

#include "Utils/Log.h"

namespace aero3d {

NativeVFile::NativeVFile(FileHandle handle, std::string& virtualPath)
    : m_Handle(handle), m_VirtualPath(virtualPath), m_Opened(false)
{
    if (m_Handle != -1) {
        m_Opened = true;

        off_t current = lseek(m_Handle, 0, SEEK_CUR);
        off_t end = lseek(m_Handle, 0, SEEK_END);
        if (end != -1) {
            m_Length = static_cast<uint64_t>(end);
        } else {
            m_Length = 0;
        }
        lseek(m_Handle, current, SEEK_SET);
    }
}

NativeVFile::~NativeVFile()
{
    if (m_Handle != -1)
        close(m_Handle);

    if (m_Data) delete[] m_Data;
}

void NativeVFile::ReadBytes(void* buffer, size_t size, size_t start)
{
    if (!buffer) {
        LogErr(ERROR_INFO, "Buffer is nullptr in file: %s", m_VirtualPath.c_str());
        return;
    }

    if (start + size > m_Length) {
        LogErr(ERROR_INFO, "Size is bigger than length in file: %s", m_VirtualPath.c_str());
        size = static_cast<size_t>(m_Length - start);
    }

    if (lseek(m_Handle, static_cast<off_t>(start), SEEK_SET) == -1) {
        LogErr(ERROR_INFO, "Failed to set file pointer: %s", m_VirtualPath.c_str());
        return;
    }

    ssize_t bytesRead = read(m_Handle, buffer, size);
    if (bytesRead < 0) {
        LogErr(ERROR_INFO, "Failed to read bytes from file: %s", m_VirtualPath.c_str());
    }
}

std::string NativeVFile::ReadString()
{
    std::string result(static_cast<size_t>(m_Length), '\0');

    if (lseek(m_Handle, 0, SEEK_SET) == -1) {
        LogErr(ERROR_INFO, "Failed to reset file pointer in file: %s", m_VirtualPath.c_str());
        return "";
    }

    ssize_t bytesRead = read(m_Handle, result.data(), m_Length);
    if (bytesRead != static_cast<ssize_t>(m_Length)) {
        LogErr(ERROR_INFO, "Failed to read string from file: %s", m_VirtualPath.c_str());
        return "";
    }

    return result;
}

void NativeVFile::Truncate(size_t pos)
{
    if (pos > static_cast<size_t>(std::numeric_limits<off_t>::max())) {
        LogErr(ERROR_INFO, "Pos is bigger than max value: %s", m_VirtualPath.c_str());
        return;
    }

    if (ftruncate(m_Handle, static_cast<off_t>(pos)) == -1) {
        LogErr(ERROR_INFO, "Failed to truncate file: %s", m_VirtualPath.c_str());
        return;
    }

    m_Length = pos;
}

void NativeVFile::WriteBytes(void* data, size_t size, size_t start)
{
    if (!data || size == 0) {
        LogErr(ERROR_INFO, "Data pointer or size is incorrect in file: %s", m_VirtualPath.c_str());
        return;
    }

    if (start > std::numeric_limits<uint64_t>::max() - size) {
        LogErr(ERROR_INFO, "Pos is too large in file: %s", m_VirtualPath.c_str());
        return;
    }

    if (lseek(m_Handle, static_cast<off_t>(start), SEEK_SET) == -1) {
        LogErr(ERROR_INFO, "Failed to set write pointer in file: %s", m_VirtualPath.c_str());
        return;
    }

    ssize_t bytesWritten = write(m_Handle, data, size);
    if (bytesWritten < 0) {
        LogErr(ERROR_INFO, "Failed to write bytes in file: %s", m_VirtualPath.c_str());
        return;
    }

    if (start + bytesWritten > m_Length) {
        m_Length = start + bytesWritten;
    }
}

void NativeVFile::Load()
{
    m_Data = new std::byte[m_Length];
    ReadBytes(m_Data, m_Length, 0);
}

void NativeVFile::Unload()
{
    delete[] m_Data;
    m_Data = nullptr;
}

bool NativeVFile::IsWritable()
{
    return true;
}

bool NativeVFile::IsOpened()
{
    return m_Opened;
}

void* NativeVFile::GetData()
{
    return m_Data;
}

uint64_t NativeVFile::GetLength() const
{
    return m_Length;
}

const std::string& NativeVFile::GetName() const
{
    return m_VirtualPath;
}

} // namespace aero3d
