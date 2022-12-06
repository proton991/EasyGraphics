#include <ezg_vk/context.hpp>
#include <ezg_vk/device.hpp>
#include <iostream>

using namespace ezg;
int main(int argc, char* argv[]) {
  if (!vk::Context::InitLoader()) {
    std::cerr << "Failed to init loader\n";
  }
  vk::Context ctx;

  vk::ContextCreateInfo ctxInfo{};
  vk::CreateContext(ctxInfo, &ctx);

  vk::Device device;
  device.SetContext(ctx);
  device.DisplayInfo();
  return 0;
}