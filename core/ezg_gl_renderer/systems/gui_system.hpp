#ifndef EASYGRAPHICS_GUI_SYSTEM_HPP
#define EASYGRAPHICS_GUI_SYSTEM_HPP
#include "base.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <vector>
#include <string>
using namespace ezg::gl;
namespace ezg::system {
struct GUIInfo {
  const std::vector<const char*>& model_list;
};
class GUISystem {
  friend class Engine;
public:
  static Ref<GUISystem> Create(GLFWwindow* glfw_window);
  GUISystem(GLFWwindow* glfw_window);
  ~GUISystem();

  void draw(const GUIInfo& info);

private:
  void begin_frame();
  void end_frame();
  void render_frame();

  int m_selected_model{0};
  bool m_rotate_model{true};
};
}  // namespace ezg::system
#endif  //EASYGRAPHICS_GUI_SYSTEM_HPP
