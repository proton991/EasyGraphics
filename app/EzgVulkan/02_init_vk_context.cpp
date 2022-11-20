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

  auto windowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

  SDL_Window* window = SDL_CreateWindow("EZGEngine", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, 800, 600, windowFlags);
  vk::ContextCreateInfo ctxInfo{};
  ctxInfo.window = window;
  vk::CreateContext(ctxInfo, &ctx);
  SDL_DestroyWindow(window);
  return 0;
}