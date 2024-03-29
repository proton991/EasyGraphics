#ifndef EASYGRAPHICS_WINDOW_SYSTEM_HPP
#define EASYGRAPHICS_WINDOW_SYSTEM_HPP

#include <memory>
#include "graphics/context.hpp"

struct GLFWwindow;
namespace ezg::system {
struct Extend2D {
  int width;
  int height;
};

struct WindowConfig {
  unsigned int width;
  unsigned int height;
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
  [[nodiscard]] auto should_resize() const { return m_data.should_resize; }

  void resize() { m_data.should_resize = false; }

  [[nodiscard]] auto get_width() const { return m_data.width; }
  [[nodiscard]] auto get_height() const { return m_data.height; }
  [[nodiscard]] auto get_aspect() const { return float(m_data.width) / float(m_data.height); }

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
    bool show_cursor{true};
    bool should_resize{false};
  } m_data;
};
}  // namespace ezg::system
#endif  //EASYGRAPHICS_WINDOW_SYSTEM_HPP
