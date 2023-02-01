#ifndef EASYGRAPHICS_WINDOW_SYSTEM_HPP
#define EASYGRAPHICS_WINDOW_SYSTEM_HPP

#include "graphics/context.hpp"

struct GLFWwindow;
namespace ezg::system {
struct Extend2D {
  int width;
  int height;
};

struct WindowConfig {
  int width;
  int height;
  int resizable;
  const char* title;
  int major_version;
  int minor_version;
};

class Window {
public:
  static WindowConfig default_config();
  explicit Window(const WindowConfig& config);
  Window(Window&&)                 = delete;
  Window& operator=(Window&&)      = delete;
  Window(const Window&)            = delete;
  Window& operator=(const Window&) = delete;
  [[nodiscard]] GLFWwindow* Handle() const { return m_window; }

  void set_glfw_callbacks();

  void wait_for_focus();

  void swap_buffers() const;

  void enable_cursor() const;

  void disable_cursor() const;

  [[nodiscard]] Extend2D get_framebuffer_size() const;

  void update();

  [[nodiscard]] auto should_close() const { return m_data.should_close; }

private:
  bool center_window();
  void destroy();

  GLFWwindow* m_window{nullptr};
  std::unique_ptr<gl::Context> m_context;

  struct WindowData {
    // size
    int width;
    int height;

    // status
    bool should_close{false};
    bool show_cursor{false};
    bool should_resize{false};
  } m_data;
};
}  // namespace ezg::system
#endif  //EASYGRAPHICS_WINDOW_SYSTEM_HPP
