#include "window.hpp"
#include <spdlog/spdlog.h>
#include "context.hpp"

namespace ezg::vk {
namespace spd = spdlog;
Window::Window(const WindowConfig& config) : m_width(config.width), m_height(config.height) {
  uint32_t flags = (SDL_WindowFlags)SDL_WINDOW_VULKAN;
  if (config.resizable) {
    flags |= SDL_WINDOW_RESIZABLE;
  }
  m_window =
      SDL_CreateWindow(config.title, config.posX, config.posY, config.width, config.height, flags);

  SDL_Init(SDL_INIT_VIDEO);
  // place cursor at the center of the screen
  SDL_WarpMouseInWindow(m_window, m_width / 2, m_height / 2);
}

Window::~Window() {
  if (m_window != nullptr) {
    SDL_DestroyWindow(m_window);
  }
}

std::vector<const char*> Window::GetInstanceExtensions() {
  uint32_t extCount = 0;
  SDL_Vulkan_GetInstanceExtensions(m_window, &extCount, nullptr);
  std::vector<const char*> exts(extCount);
  SDL_Vulkan_GetInstanceExtensions(m_window, &extCount, exts.data());
  return exts;
}

void Window::Poll() {
  SDL_Event e;
  while (SDL_PollEvent(&e) != 0) {
    if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
      m_shouldClose = true;
    } else if (e.type == SDL_WINDOWEVENT) {
      if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        m_width = e.window.data1;
        m_height = e.window.data2;
        spd::trace("window resized to {} X {}", m_width, m_height);
      }
    }
  }
}

WindowSurface::WindowSurface(const VkInstance& instance, SDL_Window* window) : m_instance(instance) {
  if (SDL_Vulkan_CreateSurface(window, m_instance, &m_surface) != SDL_TRUE) {
    spd::error("Failed to create vulkan surface");
  }
}

WindowSurface::~WindowSurface() {
  vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

}  // namespace ezg::vk