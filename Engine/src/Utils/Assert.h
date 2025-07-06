#ifndef AERO3D_UTILS_ASSERT_H_
#define AERO3D_UTILS_ASSERT_H_

#include "Utils/Common.h"
#include "Utils/Log.h"

namespace aero3d {

extern void Assert(const char* file, const char* func, int line, bool exp,
    const char* msg);

} // namespace aero3d

#endif // AERO3D_UTILS_ASSERT_H_