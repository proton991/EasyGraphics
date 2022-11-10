#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "engine.hpp"
#include "types.hpp"

namespace ege {

bool LoadImageFromFile(EGEngine& engine, const char* file, AllocatedImage& outImage);

AllocatedImage UploadImage(int texWidth, int texHeight, VkFormat image_format, EGEngine& engine,
                           AllocatedBuffer& stagingBuffer);
}  // namespace ege
#endif  //TEXTURE_HPP
