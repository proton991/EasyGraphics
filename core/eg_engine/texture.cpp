#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include "vulkan_helper/vk_init.hpp"

namespace ege {

bool LoadImageFromFile(EGEngine& engine, const char* file, AllocatedImage& outImage) {
  int width;
  int height;
  int channels;
  stbi_uc* pixels = stbi_load(file, &width, &height, &channels, STBI_rgb_alpha);

  if (!pixels) {
    std::cout << "Failed to load texture file " << file << std::endl;
    return false;
  }

  void* pixelPtr = pixels;

  VkDeviceSize imageSize = width * height * 4;
  VkFormat imageFormat   = VK_FORMAT_R8G8B8A8_SRGB;

  AllocatedBuffer stagingBuffer =
      engine.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO,
                          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

  void* data;
  vmaMapMemory(engine.m_allocator, stagingBuffer.m_allocation, &data);
  memcpy(data, pixelPtr, static_cast<size_t>(imageSize));
  vmaUnmapMemory(engine.m_allocator, stagingBuffer.m_allocation);

  stbi_image_free(pixels);

  outImage = UploadImage(width, height, imageFormat, engine, stagingBuffer);

  vmaDestroyBuffer(engine.m_allocator, stagingBuffer.m_buffer, stagingBuffer.m_allocation);
  std::cout << "Texture loaded successfully " << file << std::endl;

  return true;
}

AllocatedImage UploadImage(int texWidth, int texHeight, VkFormat image_format, EGEngine& engine,
                           AllocatedBuffer& stagingBuffer) {
  VkExtent3D imageExtent;
  imageExtent.width  = static_cast<uint32_t>(texWidth);
  imageExtent.height = static_cast<uint32_t>(texHeight);
  imageExtent.depth  = 1;

  VkImageCreateInfo imageInfo = vkh::init::ImageCreateInfo(
      image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

  AllocatedImage newImage;

  VmaAllocationCreateInfo imgAllocInfo{};
  imgAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  imgAllocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

  //allocate and create the image
  vmaCreateImage(engine.m_allocator, &imageInfo, &imgAllocInfo, &newImage.m_image,
                 &newImage.m_allocation, nullptr);

  //transition image to transfer-receiver
  engine.ImmediateSubmit([&](VkCommandBuffer cmd) {
    VkImageSubresourceRange range;
    range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel   = 0;
    range.levelCount     = 1;
    range.baseArrayLayer = 0;
    range.layerCount     = 1;

    VkImageMemoryBarrier imageBarrier_toTransfer{};
    imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    imageBarrier_toTransfer.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrier_toTransfer.newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier_toTransfer.image            = newImage.m_image;
    imageBarrier_toTransfer.subresourceRange = range;

    imageBarrier_toTransfer.srcAccessMask = 0;
    imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    //barrier the image into the transfer-receive layout
    engine.m_dispatchTable.fp_vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                   VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                                                   nullptr, 1, &imageBarrier_toTransfer);

    VkBufferImageCopy copyRegion{};
    copyRegion.bufferOffset      = 0;
    copyRegion.bufferRowLength   = 0;
    copyRegion.bufferImageHeight = 0;

    copyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel       = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount     = 1;
    copyRegion.imageExtent                     = imageExtent;

    //copy the buffer into the image
    engine.m_dispatchTable.fp_vkCmdCopyBufferToImage(cmd, stagingBuffer.m_buffer, newImage.m_image,
                                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                                     &copyRegion);

    VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

    imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    //barrier the image into the shader readable layout
    engine.m_dispatchTable.fp_vkCmdPipelineBarrier(
        cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
        0, nullptr, 1, &imageBarrier_toReadable);
  });

  engine.m_mainDestructionQueue.PushFunction([=, &engine]() {
    vmaDestroyImage(engine.m_allocator, newImage.m_image, newImage.m_allocation);
  });
  return newImage;
}
}  // namespace ege