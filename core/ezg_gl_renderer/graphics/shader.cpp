#include "shader.hpp"
#include <utility>
#include "log.hpp"
#include "managers/resource_manager.hpp"

namespace ezg::gl {

std::unordered_map<std::string, int> ShaderStage::Name2GL_ENUM = {{"vertex", GL_VERTEX_SHADER},
                                                                  {"fragment", GL_FRAGMENT_SHADER},
                                                                  {"geometry", GL_GEOMETRY_SHADER},
                                                                  {"compute", GL_COMPUTE_SHADER}};

bool compile_shader(GLuint id, const std::string& code) {
  GLint success{GL_FALSE};
  GLint log_len{-1};
  const char* shader_src = code.c_str();
  glShaderSource(id, 1, &shader_src, nullptr);
  glCompileShader(id);
  glGetShaderiv(id, GL_COMPILE_STATUS, &success);

  if (success == GL_FALSE) {
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_len);
    std::vector<GLchar> info_log(log_len);
    glGetShaderInfoLog(id, log_len, nullptr, info_log.data());
    spd::error("Failed to compile shader: {}", std::string(info_log.begin(), info_log.end()));
  }

  return success == GL_TRUE;
}

bool link_program(GLuint id) {
  GLint success{GL_FALSE};
  GLint log_len{-1};

  glLinkProgram(id);
  glGetProgramiv(id, GL_LINK_STATUS, &success);

  if (success == GL_FALSE) {
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_len);
    std::vector<GLchar> info_log(log_len);
    glGetProgramInfoLog(id, log_len, nullptr, info_log.data());
    spd::error("Failed to link program: {}", std::string(info_log.begin(), info_log.end()));
    return false;
  }

  glValidateProgram(id);
  glGetProgramiv(id, GL_VALIDATE_STATUS, &success);

  if (success == GL_FALSE) {
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_len);
    std::vector<GLchar> info_log(log_len);
    glGetProgramInfoLog(id, log_len, nullptr, info_log.data());
    spd::error("Failed to valid program: {}", std::string(info_log.begin(), info_log.end()));
    return false;
  }
  return success == GL_TRUE;
}

Ref<ShaderProgram> ShaderProgramFactory::create_shader_program(
    const ShaderProgramCreateInfo& info) {
  spd::trace("Building shader program {}", info.name);
  std::vector<GLuint> shader_ids;
  shader_ids.reserve(info.stages.size());
  bool success = true;
  for (const auto& stage : info.stages) {
    auto id{glCreateShader(ShaderStage::Name2GL_ENUM.at(stage.type))};
    auto shader_code = ResourceManager::load_shader_source(stage.file_path);
    if (shader_code.empty()) {
      return nullptr;
    }
    if (!compile_shader(id, shader_code)) {
      success = false;
      break;
    }
    shader_ids.emplace_back(id);
  }

  if (!success) {
    for (const auto id : shader_ids) {
      glDeleteShader(id);
    }
    return nullptr;
  }

  // link program and validate
  GLuint program_id{glCreateProgram()};
  for (const auto id : shader_ids) {
    glAttachShader(program_id, id);
  }
  if (!link_program(program_id)) {
    for (const auto id : shader_ids) {
      glDetachShader(program_id, id);
      glDeleteShader(id);
    }
    return nullptr;
  }
  for (const auto id : shader_ids) {
    glDeleteShader(id);
  }
  return CreateRef<ShaderProgram>(info.name, program_id);
}

ShaderProgram::ShaderProgram(std::string name, GLuint id) : m_id(id), m_name(std::move(name)) {
  get_uniforms();
}

ShaderProgram::~ShaderProgram() {
  if (m_id != 0) {
    glDeleteProgram(m_id);
  }
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept {
  m_uniforms = std::move(other.m_uniforms);
  m_id       = other.m_id;
  m_name     = std::move(other.m_name);
  other.m_id = 0;
}

ShaderProgram& ShaderProgram::use() {
  glUseProgram(m_id);
  return *this;
}

ShaderProgram& ShaderProgram::set_uniform(const std::string& name, int value) {
  if (m_uniforms.contains(name)) {
    glUniform1i(m_uniforms.at(name), value);
  }
  return *this;
}

ShaderProgram& ShaderProgram::set_uniform(const std::string& name, float value) {
  if (m_uniforms.contains(name)) {
    glUniform1f(m_uniforms.at(name), value);
  }
  return *this;
}

ShaderProgram& ShaderProgram::set_uniform(const std::string& name, const glm::vec2& value) {
  if (m_uniforms.contains(name)) {
    glUniform2fv(m_uniforms.at(name), 1, value_ptr(value));
  }
  return *this;
}

ShaderProgram& ShaderProgram::set_uniform(const std::string& name, const glm::vec3& value) {
  if (m_uniforms.contains(name)) {
    glUniform3fv(m_uniforms.at(name), 1, value_ptr(value));
  }
  return *this;
}

ShaderProgram& ShaderProgram::set_uniform(const std::string& name, const glm::vec4& value) {
  if (m_uniforms.contains(name)) {
    glUniform4fv(m_uniforms.at(name), 1, value_ptr(value));
  }
  return *this;
}

ShaderProgram& ShaderProgram::set_uniform(const std::string& name, const glm::mat3x3& value) {
  if (m_uniforms.contains(name)) {
    glUniformMatrix3fv(m_uniforms.at(name), 1, GL_FALSE, value_ptr(value));
  }
  return *this;
}

ShaderProgram& ShaderProgram::set_uniform(const std::string& name, const glm::mat4x4& value) {
  if (m_uniforms.contains(name)) {
    glUniformMatrix4fv(m_uniforms.at(name), 1, GL_FALSE, value_ptr(value));
  } else {
    int location = glGetUniformLocation(m_id, name.data());
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(value));
  }
  return *this;
}

void ShaderProgram::get_uniforms() {
  int num_uniforms = 0;
  glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &num_uniforms);
  for (auto i = 0U; i < num_uniforms; i++) {
    auto name_len = -1, size = -1;
    GLenum type = GL_ZERO;
    char name[128];
    glGetActiveUniform(m_id, i, sizeof(name) - 1, &name_len, &size, &type, name);
    name[name_len] = 0;

    const auto nameStr = std::string(name);
    m_uniforms.try_emplace(nameStr, glGetUniformLocation(m_id, name));
  }
}
}  // namespace ezg::gl