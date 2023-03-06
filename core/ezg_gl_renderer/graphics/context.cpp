// glad must be included before GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include "log.hpp"
#include "context.hpp"

namespace ezg::gl {
static const std::unordered_map<GLenum, const char*> SourceEnumToString = {
    {GL_DEBUG_SOURCE_API, "API"},
    {GL_DEBUG_SOURCE_WINDOW_SYSTEM, "WINDOW_SYSTEM"},
    {GL_DEBUG_SOURCE_SHADER_COMPILER, "SHADER_COMPILER"},
    {GL_DEBUG_SOURCE_THIRD_PARTY, "THIRD_PARTY"},
    {GL_DEBUG_SOURCE_APPLICATION, "APPLICATION"},
    {GL_DEBUG_SOURCE_OTHER, "OTHER"}};

static const std::unordered_map<GLenum, const char*> TypeEnumToString = {
    {GL_DEBUG_TYPE_ERROR, "ERROR"},
    {GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "DEPRECATED_BEHAVIOR"},
    {GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "UNDEFINED_BEHAVIOR"},
    {GL_DEBUG_TYPE_PORTABILITY, "PORTABILITY"},
    {GL_DEBUG_TYPE_PERFORMANCE, "PERFORMANCE"},
    {GL_DEBUG_TYPE_OTHER, "OTHER"}};

static const std::unordered_map<GLenum, const char*> SeverityEnumToString = {
    {GL_DEBUG_SEVERITY_HIGH, "HIGH"},
    {GL_DEBUG_SEVERITY_MEDIUM, "MEDIUM"},
    {GL_DEBUG_SEVERITY_LOW, "LOW"},
    {GL_DEBUG_SEVERITY_NOTIFICATION, "NOTIFICATION"}};

// List of message type to ignore for GL Debug Output
static const std::vector<std::tuple<GLenum, GLenum, GLenum>> IgnoreList = {
    std::make_tuple(GL_DONT_CARE, GL_DONT_CARE,
                    GL_DEBUG_SEVERITY_NOTIFICATION)  // Ignore all notifications
};
void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                    const GLchar* message, GLvoid* userParam);
Context::Context(GLFWwindow* window_handle) {
  glfwMakeContextCurrent(window_handle);
  int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  if (!status) {
    spdlog::error("Failed to initialize glad!");
  }
  spdlog::trace("Displaying OpenGL Info:");
  spdlog::info("Vendor: {}", std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR))));
  spdlog::info("Renderer: {}",
               std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER))));
  spdlog::info("Version: {}", std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))));
  init_debug();
}

void Context::init_debug() {
  glDebugMessageCallback((GLDEBUGPROCARB)debug_callback, nullptr);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

  for (const auto& tuple : IgnoreList) {
    glDebugMessageControl(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple), 0, nullptr,
                          GL_FALSE);
  }
}

void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                    const GLchar* message, GLvoid* user_param) {
  const auto find_str = [](GLenum value, const auto& map) {
    const auto it = map.find(value);
    if (it == end(map)) {
      return "UNDEFINED";
    }
    return (*it).second;
  };

  const auto source_str   = find_str(source, SourceEnumToString);
  const auto type_str     = find_str(type, TypeEnumToString);
  const auto severity_str = find_str(severity, SeverityEnumToString);
  spdlog::debug("[GLDebug]: [source={} type={} severity{} id={}]", source_str, type_str, severity_str, id);

}

}  // namespace ezg::gl