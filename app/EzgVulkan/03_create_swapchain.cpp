#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <ezg_vk/context.hpp>
#include <ezg_vk/device.hpp>
#include <ezg_vk/swapchain.hpp>
#include <ezg_vk/image.hpp>
#include <iostream>
#include <vector>

using namespace ezg;
int main(int argc, char* argv[]) {
  if (!vk::Context::InitLoader()) {
    std::cerr << "Failed to init loader\n";
  }
  vk::Context ctx;
  SDL_Init(SDL_INIT_VIDEO);
  wsi::SDL2WindowConfig config{};
  config.flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
  config.height = 600;
  config.width = 800;

  auto* platform = new wsi::SDL2Platform();
  platform->Init(config);

  vk::ContextCreateInfo ctxInfo{};
  //  ctxInfo.platform = platform;
  std::vector<const char*> instExts = platform->GetInstanceExtensions();
  std::vector<const char*> deviceExts = platform->GetDeviceExtensions();
  ctxInfo.enabledInstExtCount = instExts.size();
  ctxInfo.ppEnabledInstExts = instExts.data();
  ctxInfo.enabledDeviceExtCount = deviceExts.size();
  ctxInfo.ppEnabledDeviceExts = deviceExts.data();
  vk::CreateContext(ctxInfo, &ctx);

  vk::Device device;
  device.SetContext(ctx);
  device.DisplayInfo();

  vk::Swapchain swapchain{platform, ctx.GetVkInstance(), &device, 800, 600, true};
  vk::ImageCreateInfo imageInfo = {
      .format = VK_FORMAT_D32_SFLOAT,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .aspect = VK_IMAGE_ASPECT_DEPTH_BIT,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .vmaAllocFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
      .name = "depth image",
      .imageExtent = {
            .width = 800,
            .height = 600
      }
  };
  vk::Image image(device, imageInfo);
  return 0;
}