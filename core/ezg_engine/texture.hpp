#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "engine.hpp"
#include "types.hpp"

namespace ezg {

bool LoadImageFromFile(EGEngine& engine, const char* file, AllocatedImage& outImage);

AllocatedImage UploadImage(int texWidth, int texHeight, VkFormat image_format, EGEngine& engine,
                           AllocatedBuffer& stagingBuffer);
}  // namespace ezg
#endif  //TEXTURE_HPP
