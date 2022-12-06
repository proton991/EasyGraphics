#ifndef EASYGRAPHICS_WINDOW_HPP
#define EASYGRAPHICS_WINDOW_HPP

#include <SDL2/SDL_vulkan.h>
#include <string>
#include "common.hpp"

namespace ezg::vk {
class Instance;
struct WindowConfig {
  const char* title{"Default"};
  bool resizable{false};
  int posX{SDL_WINDOWPOS_UNDEFINED};
  int posY{SDL_WINDOWPOS_UNDEFINED};
  int width{800};
  int height{600};
};
class Window {
public:
  NO_COPY(Window)
  NO_MOVE(Window)

  explicit Window(const WindowConfig& config);
  ~Window();

  [[nodiscard]] SDL_Window* Handle() const { return m_window; }
  std::vector<const char*> GetDeviceExtensions() { return {"VK_KHR_swapchain"}; }
  std::vector<const char*> GetInstanceExtensions();

  void Poll();

  [[nodiscard]] bool ShouldClose() const { return m_shouldClose; }
  [[nodiscard]] bool IsResized() const { return m_resized; }

  [[nodiscard]] uint32_t GetWidth() const { return m_width; }
  [[nodiscard]] uint32_t GetHeight() const { return m_height; }

private:
  SDL_Window* m_window{nullptr};
  uint32_t m_width;
  uint32_t m_height;
  bool m_shouldClose{false};
  bool m_resized{false};
};

class WindowSurface {
public:
  NO_COPY(WindowSurface)
  NO_MOVE(WindowSurface)

  WindowSurface(const VkInstance& instance, SDL_Window* window);
  ~WindowSurface();

  [[nodiscard]] VkSurfaceKHR Handle() const { return m_surface; }
private:
  const VkInstance& m_instance;
  VkSurfaceKHR m_surface;
};
}  // namespace ezg::vk
#endif  //EASYGRAPHICS_WINDOW_HPP
