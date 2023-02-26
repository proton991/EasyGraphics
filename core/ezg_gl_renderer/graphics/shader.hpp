//
// Created by cxy on 2023/1/24.
//

#ifndef EASYGRAPHICS_SHADER_HPP
#define EASYGRAPHICS_SHADER_HPP

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include <glad/glad.h>

namespace ezg::gl {
bool compile_shader(GLuint id, const std::string& code);
bool link_program(GLuint id);

struct ShaderStage {
  ShaderStage() noexcept = default;

  ShaderStage(std::string path, std::string type)
      : file_path{std::move(path)}, type{std::move(type)} {}

  std::string file_path;
  std::string type;
  static std::unordered_map<std::string, int> Name2GL_ENUM;

};

struct ShaderProgramCreateInfo {
  const std::string& name;
  const std::vector<ShaderStage>& stages;
};

class ShaderProgram {
public:
  ShaderProgram(std::string name, GLuint id);
  ~ShaderProgram();

  ShaderProgram(ShaderProgram&& other) noexcept;

  void use() const;

  ShaderProgram& set_uniform(const std::string& name, int value);
  ShaderProgram& set_uniform(const std::string& name, float value);
  ShaderProgram& set_uniform(const std::string& name, const glm::vec2& value);
  ShaderProgram& set_uniform(const std::string& name, const glm::vec3& value);
  ShaderProgram& set_uniform(const std::string& name, const glm::vec4& value);
  ShaderProgram& set_uniform(const std::string& name, const glm::mat3x3& value);
  ShaderProgram& set_uniform(const std::string& name, const glm::mat4x4& value);

  ShaderProgram(const ShaderProgram&)            = delete;
  ShaderProgram& operator=(ShaderProgram&&)      = delete;
  ShaderProgram& operator=(const ShaderProgram&) = delete;

  auto get_location(const std::string& name) const { return m_uniforms.at(name); }
private:
  void get_uniforms();
  std::unordered_map<std::string, int> m_uniforms;  // <name, location>
  GLuint m_id{0};
  std::string m_name;
};

class ShaderProgramFactory {
public:
  static std::optional<ShaderProgram> create_shader_program(const ShaderProgramCreateInfo& info);
};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_SHADER_HPP
