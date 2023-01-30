#ifndef EASYGRAPHICS_CONTEXT_HPP
#define EASYGRAPHICS_CONTEXT_HPP
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>
#include <string>
namespace ezg::gl {

class Context {
public:
  explicit Context(GLFWwindow* window_handle) {
    glfwMakeContextCurrent(window_handle);
    int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (!status) {
      spdlog::error("Failed to initialize glad!");
    }
    spdlog::info("OpenGL Info:");
    spdlog::info("  Vendor: {}",
                 std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR))));
    spdlog::info("  Renderer: {}",
                 std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER))));
    spdlog::info("  Version: {}",
                 std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))));
  }

  Context(Context&&)                 = delete;
  Context(const Context&)            = delete;
  Context& operator=(Context&&)      = delete;
  Context& operator=(const Context&) = delete;
};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_CONTEXT_HPP
