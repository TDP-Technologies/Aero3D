#ifndef AERO3D_UTILS_IMAGELOADER_H_
#define AERO3D_UTILS_IMAGELOADER_H_

#include <string>
#include <cstdint>
#include <vector>

#include <Graphics/Resources.h>

namespace aero3d {

struct ImageData
{
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    TextureFormat format;
    std::vector<uint8_t> pixels;
};

class ImageLoader
{
public:
    static ImageData LoadImage(std::string path);

};

} // namespace aero3d

#endif // AERO3D_UTILS_IMAGELOADER_H_