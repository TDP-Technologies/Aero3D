#ifndef AERO3D_CORE_CONFIGURATION_H_
#define AERO3D_CORE_CONFIGURATION_H_

#include <map>
#include <string>
#include <memory>

#include "IO/VFile.h"
#include "Utils/Common.h"
#include "Graphics/RenderAPI.h"

namespace aero3d {

class Configuration
{
public:
    Configuration() = default;
    ~Configuration();

    void Open(std::string path);

    std::string GetValue(std::string key);

private:
    std::map<std::string, std::string> m_ConfigMap {};
    Ref<VFile> m_ConfigFile = nullptr;

};

} // namespace aero3d

#endif // AERO3D_CORE_CONFIGURATION_H_