#ifndef EASYGRAPHICS_SHADER_HPP
#define EASYGRAPHICS_SHADER_HPP

#include <volk.h>
#include <string>
#include "common.hpp"

namespace ezg::vk {
class Device;
class Shader {
public:
  NO_COPY(Shader)
  Shader(const Device* device, const VkShaderStageFlagBits stage, const std::string& name,
         const std::string& path, const std::string& entryPoint = "main");

  Shader(Shader&&) noexcept;

  ~Shader();

  Shader& operator=(Shader&&) = delete;
  [[nodiscard]] VkShaderStageFlagBits Stage() const;
  [[nodiscard]] const std::string& Name() const;
  [[nodiscard]] const std::string& EntryPoint() const;
  [[nodiscard]] const VkShaderModule Module() const;

private:
  const Device* m_device;
  VkShaderModule m_module;
  VkShaderStageFlagBits m_stage;
  std::string m_name;
  std::string m_entryPoint;
};
}  // namespace ezg::vk

#endif  //EASYGRAPHICS_SHADER_HPP
