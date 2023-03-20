#include <ezg_vk_hpp/context.hpp>
#include <ezg_vk_hpp/device.hpp>
using namespace ezg;
int main(int argc, char* argv[]) {
  vulkan::Context ctx;
  vulkan::ContextCreateInfo context_ci{{}, {}, nullptr};
  ctx.InitInstanceAndDevice(context_ci);
  vulkan::Device device;
  device.SetContext(ctx);
  return 0;
}