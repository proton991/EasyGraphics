#include <ezg_vk_hpp/context.hpp>
#include <ezg_vk_hpp/logging.hpp>
using namespace ezg;
int main(int argc, char* argv[]) {
  vulkan::Context ctx;
  ctx.CreateInstance(nullptr, 0);
  return 0;
}