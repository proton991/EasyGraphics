#include <ezg_vk/context.hpp>
#include <ezg_vk/device.hpp>
#include <ezg_vk/swapchain.hpp>
#include <ezg_vk/image.hpp>
#include <ezg_vk/window.hpp>
#include <iostream>
#include <vector>

using namespace ezg;
int main(int argc, char* argv[]) {
  spd::set_level(spd::level::trace);
  vk::WindowConfig windowConfig = {
      .title     = "Window Test",
      .resizable = true,
      .width     = 800,
      .height    = 600,
  };
  vk::Window window {windowConfig};
  if (!vk::Context::InitLoader()) {
    std::cerr << "Failed to init loader\n";
  }
  vk::Context ctx;

  vk::ContextCreateInfo ctxInfo{};
  std::vector<const char*> instExts = window.GetInstanceExtensions();
  std::vector<const char*> deviceExts = window.GetDeviceExtensions();
  ctxInfo.enabledInstExtCount = instExts.size();
  ctxInfo.ppEnabledInstExts = instExts.data();
  ctxInfo.enabledDeviceExtCount = deviceExts.size();
  ctxInfo.ppEnabledDeviceExts = deviceExts.data();
  vk::CreateContext(ctxInfo, &ctx);

  vk::Device device;
  device.SetContext(ctx);
  device.DisplayInfo();
  vk::WindowSurface surface{ctx.GetVkInstance(), window.Handle()};
  vk::Swapchain swapchain{&device, surface, 800, 600, true};
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