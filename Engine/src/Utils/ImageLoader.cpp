#include "Utils/ImageLoader.h"

#include <cstring>

#include <stb_image/std_image.h>

#include "Utils/Log.h"
#include "IO/VFS.h"

namespace aero3d {

ImageData ImageLoader::LoadImage(std::string path) 
{
    ImageData imageData{};

    int texWidth, texHeight, texChannels;

    Ref<VFile> file = VFS::ReadFile(path);
    file->Load();

    const stbi_uc* data = reinterpret_cast<const stbi_uc*>(file->GetData());
    int size = static_cast<int>(file->GetLength());

    stbi_uc* pixels = stbi_load_from_memory(data, size, 
        &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    if (!pixels) 
    {
        LogErr(ERROR_INFO, "Failed to load image: %s", path.c_str());
        return imageData;
    }

    imageData.width = static_cast<uint32_t>(texWidth);
    imageData.height = static_cast<uint32_t>(texHeight);
    imageData.channels = 4;
    imageData.format = TextureFormat::RGBA8;
    imageData.pixels.resize(imageData.width * imageData.height * imageData.channels);
    std::memcpy(imageData.pixels.data(), pixels, imageData.pixels.size());

    stbi_image_free(pixels);
    return imageData;
}

} // namespace aero3d