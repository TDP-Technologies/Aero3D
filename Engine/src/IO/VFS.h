#ifndef AERO3D_IO_VFS_H_
#define AERO3D_IO_VFS_H_

#include <string>
#include <vector>
#include <memory>

#include "IO/VFDirectory.h"
#include "IO/VFile.h"
#include "Utils/Common.h"

namespace aero3d {

class VFS {
public:
    static void Init();
    static void Shutdown();

    static void Mount(std::string virtualPath, std::string mounPoint, 
        DirType type = DirType::NATIVE, bool appendToFront = false);

    static Ref<VFile> ReadFile(std::string path);

private:
    static std::vector<Scope<VFDirectory>> s_Dirs;
    static Scope<VFDirectory> s_DefaultDir;

};

} // namespace aero3d

#endif // AERO3D_IO_VFS_H_