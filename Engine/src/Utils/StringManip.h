#ifndef AERO3D_UTILS_STRINGMANIP_H_
#define AERO3D_UTILS_STRINGMANIP_H_

#include <string>

#include "Utils/Common.h"

namespace aero3d {

extern std::string GetPathAfter(const char* path, const char* after);
extern std::string ExtractClassAndFunctionName(const char* prettyFunctionName);

} // namespace aero3d

#endif // AERO3D_UTILS_STRINGMANIP_H_