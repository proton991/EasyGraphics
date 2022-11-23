#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <ezg_vk/context.hpp>
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
  ctxInfo.platform = platform;
  vk::CreateContext(ctxInfo, &ctx);
  return 0;
}