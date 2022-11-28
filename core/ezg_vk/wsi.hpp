#ifndef EASYGRAPHICS_WSI_HPP
#define EASYGRAPHICS_WSI_HPP

#include <SDL2/SDL_vulkan.h>
#include <volk.h>
#include <vector>

namespace ezg::wsi {
struct SDL2WindowConfig {
  const char* title;
  int posX{SDL_WINDOWPOS_UNDEFINED};
  int posY{SDL_WINDOWPOS_UNDEFINED};
  int width;
  int height;
  uint32_t flags;
};
class Platform {
public:
  virtual ~Platform() = default;

  virtual VkSurfaceKHR CreateSurface(VkInstance instance) const = 0 ;

  virtual void DestroySurface(VkInstance instance, VkSurfaceKHR surface);

  virtual std::vector<const char*> GetInstanceExtensions() = 0;

  virtual std::vector<const char*> GetDeviceExtensions() { return {"VK_KHR_swapchain"}; }
};

class SDL2Platform : public Platform {
public:
  void Init(const SDL2WindowConfig& windowConfig);

  ~SDL2Platform() override;

  VkSurfaceKHR CreateSurface(VkInstance instance) const override;

  std::vector<const char*> GetInstanceExtensions() override;

  [[nodiscard]] int GetWidth() const { return m_width; }
  [[nodiscard]] int GetHeight() const { return m_height; }

private:
  SDL_Window* m_window{nullptr};
  int m_width;
  int m_height;
};
}  // namespace ezg::wsi

#endif  //EASYGRAPHICS_WSI_HPP
