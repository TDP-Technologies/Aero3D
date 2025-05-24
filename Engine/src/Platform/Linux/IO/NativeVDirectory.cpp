#include "IO/NativeVFDirectory.h"

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
}

bool NativeVFDirectory::FileExists(std::string& path)
{
}

} // namespace aero3d