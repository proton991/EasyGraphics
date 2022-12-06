#include <ezg_vk/window.hpp>
#include <ezg_vk/context.hpp>
#include <ezg_vk/device.hpp>
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
    spd::error("Failed to init loader");
  }
  vk::Context ctx;

  vk::ContextCreateInfo ctxInfo{};
  //  ctxInfo.platform = platform;
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
  while (!window.ShouldClose()) {
    window.Poll();
  }
  return 0;
}