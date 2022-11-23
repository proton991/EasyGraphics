#include "wsi.hpp"
#include <volk.h>

namespace ezg::wsi {
void Platform::DestroySurface(VkInstance instance, VkSurfaceKHR surface) {
  vkDestroySurfaceKHR(instance, surface, nullptr);
}
void SDL2Platform::Init(const SDL2WindowConfig& config) {
  m_window = SDL_CreateWindow(config.title, config.posX, config.posY, config.width, config.height,
                              config.flags);
}
VkSurfaceKHR SDL2Platform::CreateSurface(VkInstance instance) {
  VkSurfaceKHR surface;
  SDL_Vulkan_CreateSurface(m_window, instance, &surface);
  return surface;
}

std::vector<const char*> SDL2Platform::GetInstanceExtensions() {
  uint32_t extCount = 0;
  SDL_Vulkan_GetInstanceExtensions(m_window, &extCount, nullptr);
  std::vector<const char*> exts(extCount);
  SDL_Vulkan_GetInstanceExtensions(m_window, &extCount, exts.data());
  return exts;
}

SDL2Platform::~SDL2Platform() {
  if (m_window != nullptr) {
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
  }
}

}  // namespace ezg::wsi