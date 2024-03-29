#include "window_system.hpp"
#include <GLFW/glfw3.h>
#include "log.hpp"
#include "input_system.hpp"

namespace ezg::system {

static void glfw_error_call_back(int code, const char* msg) {
  spd::error("GLFW error [{}]: {}", code, msg);
}

WindowConfig Window::default_config() {
  WindowConfig config{};
  config.width         = 800;
  config.height        = 800;
  config.major_version = 4;
  config.minor_version = 5;
  config.resizable     = GL_FALSE;
  config.title         = "OpenGL Renderer";

  return config;
}

void Window::set_glfw_callbacks() {
  const auto resize_callback = [](GLFWwindow* w, int width, int height) {
    auto* window_data          = static_cast<WindowData*>(glfwGetWindowUserPointer(w));
    window_data->width         = width;
    window_data->height        = height;
    window_data->should_resize = true;
    spd::trace("Window resized to {} x {}", width, height);
  };
  glfwSetWindowSizeCallback(m_window, resize_callback);

  const auto key_callback = [](GLFWwindow* w, auto key, auto scancode, auto action, auto mode) {
    if (key < 0 || key > GLFW_KEY_LAST) {
      return;
    }
    switch (action) {
      case GLFW_PRESS:
        KeyboardMouseInput::GetInstance().press_key(key);
        break;
      case GLFW_RELEASE:
        KeyboardMouseInput::GetInstance().release_key(key);
        break;
      default:
        break;
    }
  };
  glfwSetKeyCallback(m_window, key_callback);

  const auto cursor_pos_callback = [](GLFWwindow* w, auto xPos, auto yPos) {
    KeyboardMouseInput::GetInstance().set_cursor_pos(xPos, yPos);
  };
  glfwSetCursorPosCallback(m_window, cursor_pos_callback);

  auto mouse_button_callback = [](GLFWwindow* window, int button, int action, int mods) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) {
      return;
    }
    switch (action) {
      case GLFW_PRESS:
        KeyboardMouseInput::GetInstance().press_mouse_button(button);
        break;
      case GLFW_RELEASE:
        KeyboardMouseInput::GetInstance().release_mouse_button(button);
        break;
      default:
        break;
    }
  };
  glfwSetMouseButtonCallback(m_window, mouse_button_callback);
}

Window::Window(const WindowConfig& config) {
  m_data.width  = config.width;
  m_data.height = config.height;

  if (!glfwInit()) {
    spd::error("Failed to initialize GLFW!");
    abort();
  }
  glfwSetErrorCallback(glfw_error_call_back);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config.major_version);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config.minor_version);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, config.resizable);

  m_window = glfwCreateWindow(m_data.width, m_data.height, config.title, nullptr, nullptr);

  if (!m_window) {
    spd::error("Failed to create GLFW window!");
    abort();
  }
  // initialize context
  m_context = std::make_unique<gl::Context>(m_window);

  glfwSetWindowUserPointer(m_window, &m_data);

  center_window();
  // setup input callbacks
  set_glfw_callbacks();
}

void Window::wait_for_focus() {
  int current_width  = 0;
  int current_height = 0;

  do {
    glfwWaitEvents();
    glfwGetFramebufferSize(m_window, &current_width, &current_height);
  } while (current_width == 0 || current_height == 0);

  m_data.width  = current_width;
  m_data.height = current_height;
}

void Window::swap_buffers() const {
  glfwSwapBuffers(m_window);
}

void Window::enable_cursor() const {
  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::disable_cursor() const {
  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

Extend2D Window::get_framebuffer_size() const {
  return {m_data.width, m_data.height};
}
void Window::update() {
  glfwPollEvents();
  if (KeyboardMouseInput::GetInstance().is_key_pressed(GLFW_KEY_TAB)) {
    m_data.show_cursor = !m_data.show_cursor;
    if (m_data.show_cursor) {
      enable_cursor();
    } else {
      disable_cursor();
    }
  }
  if (KeyboardMouseInput::GetInstance().is_key_pressed(GLFW_KEY_ESCAPE) ||
      glfwWindowShouldClose(m_window)) {
    m_data.should_close = true;
    glfwSetWindowShouldClose(m_window, GL_TRUE);
  }
}

bool Window::center_window() {
  int sx = 0, sy = 0;
  int px = 0, py = 0;
  int mx = 0, my = 0;
  int monitor_count = 0;
  int best_area     = 0;
  int final_x = 0, final_y = 0;

  glfwGetWindowSize(m_window, &sx, &sy);
  glfwGetWindowPos(m_window, &px, &py);

  // Iterate throug all monitors
  GLFWmonitor** m = glfwGetMonitors(&monitor_count);
  if (!m)
    return false;

  for (int j = 0; j < monitor_count; ++j) {

    glfwGetMonitorPos(m[j], &mx, &my);
    const GLFWvidmode* mode = glfwGetVideoMode(m[j]);
    if (!mode)
      continue;

    // Get intersection of two rectangles - screen and window
    int minX = std::max(mx, px);
    int minY = std::max(my, py);

    int maxX = std::min(mx + mode->width, px + sx);
    int maxY = std::min(my + mode->height, py + sy);

    // Calculate area of the intersection
    int area = std::max(maxX - minX, 0) * std::max(maxY - minY, 0);

    // If its bigger than actual (window covers more space on this monitor)
    if (area > best_area) {
      // Calculate proper position in this monitor
      final_x = mx + (mode->width - sx) / 2;
      final_y = my + (mode->height - sy) / 2;

      best_area = area;
    }
  }

  // We found something
  if (best_area)
    glfwSetWindowPos(m_window, final_x, final_y);

  // Something is wrong - current window has NOT any intersection with any monitors. Move it to the default one.
  else {
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (primary) {
      const GLFWvidmode* desktop = glfwGetVideoMode(primary);

      if (desktop)
        glfwSetWindowPos(m_window, (desktop->width - sx) / 2, (desktop->height - sy) / 2);
      else
        return false;
    } else
      return false;
  }

  return true;
}

void Window::destroy() {
  glfwDestroyWindow(m_window);
  glfwTerminate();
}
}  // namespace ezg::system