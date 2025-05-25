#include "IO/NativeVFDirectory.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include <memory>

#include "IO/NativeVFile.h"
#include "Utils/StringManip.h"
#include "Utils/Log.h"

#define A3D_RESOLVE_NATIVE_PATH(p) (m_MountPoint + p)

namespace aero3d {

NativeVFDirectory::NativeVFDirectory(std::string virtualPath, std::string mountPoint)
{
    m_VirtualPath = virtualPath;
    m_MountPoint = mountPoint;
}

NativeVFDirectory::~NativeVFDirectory()
{
}

Ref<VFile> NativeVFDirectory::OpenFile(std::string& path)
{
    std::string fullPath = A3D_RESOLVE_NATIVE_PATH(path);

    int fd = open(fullPath.c_str(), O_RDWR);
    if (fd == -1)
    {
        LogErr(ERROR_INFO, "Failed to open file: %s, errno: %d (%s)", path.c_str(), errno, strerror(errno));
        return nullptr;
    }

    return std::make_shared<NativeVFile>(fd, path);
}

bool NativeVFDirectory::FileExists(std::string& path)
{
    std::string fullPath = A3D_RESOLVE_NATIVE_PATH(path);

    struct stat st;
    if (stat(fullPath.c_str(), &st) == 0)
    {
        return !S_ISDIR(st.st_mode);
    }
    return false;
}

} // namespace aero3d
