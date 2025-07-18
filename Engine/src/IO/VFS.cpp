#include "IO/VFS.h"

#include <memory>

#include "IO/NativeVFDirectory.h"
#include "Utils/Common.h"
#include "Utils/Assert.h"
#include "Utils/Log.h"

namespace aero3d {

std::vector<Scope<VFDirectory>> VFS::s_Dirs = {};
Scope<VFDirectory> VFS::s_DefaultDir = std::make_unique<NativeVFDirectory>("", "");

void VFS::Mount(std::string virtualPath, std::string mountPoint, DirType type, bool appendToFront)
{
    switch (type)
    {
    case DirType::NATIVE:
    {
        if (appendToFront)
        {
            s_Dirs.insert(s_Dirs.begin(), std::make_unique<NativeVFDirectory>(virtualPath, mountPoint));
        }
        else
        {
            s_Dirs.emplace_back(std::make_unique<NativeVFDirectory>(virtualPath, mountPoint));
        }
        break;
    }
    default: Assert(ERROR_INFO, false, "Unknown DirType!");
    }
}

Ref<VFile> VFS::ReadFile(std::string path)
{
    for (const auto& dir : s_Dirs)
    {
        std::string dirVirtualPath = dir->GetVirualPath();
        if (path.starts_with(dirVirtualPath))
        {
            std::string subPath = path.substr(dirVirtualPath.length());
            if (dir->FileExists(subPath))
            {
                return dir->OpenFile(subPath);
            }
        }
    }

    return s_DefaultDir->OpenFile(path);
}

} // namespace aero3d