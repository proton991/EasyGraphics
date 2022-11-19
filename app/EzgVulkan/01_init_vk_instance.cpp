#include <ezg_vk/context.hpp>
#include <iostream>
#include <vector>

using namespace ezg;
int main(int argc, char* argv[]) {
  if (!vk::Context::InitLoader()) {
    std::cerr << "Failed to init loader\n";
  }
  vk::Context ctx;
  ctx.CreateInstance();
  if (ctx.GetVkInstance() == VK_NULL_HANDLE) {
    std::cerr << "Failed to create instance\n";
  }
}