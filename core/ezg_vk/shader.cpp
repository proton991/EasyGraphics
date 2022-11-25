#include "shader.hpp"
#include <filesystem>
#include <fstream>
#include "device.hpp"

namespace ezg::vk {

Shader::Shader(const Device* device, const VkShaderStageFlagBits stage, const std::string& name,
               const std::string& path, const std::string& entryPoint)
    : m_device(device), m_stage(stage), m_name(name), m_entryPoint(entryPoint) {
  // Open stream at the end of the file to read it's size.
  std::ifstream file(path.c_str(), std::ios::ate | std::ios::binary | std::ios::in);

  if (!file) {
    spd::critical("Error: Could not open file {} !", path);
    throw std::runtime_error("Error: Could not open file " + path + "!");
  }
  // Read the size of the file
  const auto fileSize = static_cast<uint32_t>(file.tellg());
  std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
  // Set the file read position to the beginning of the file
  file.seekg(0);
  file.read((char*)buffer.data(), fileSize);
  file.close();

  VkShaderModuleCreateInfo info{};
  info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  info.pNext    = nullptr;
  info.codeSize = buffer.size() * sizeof(uint32_t);
  info.pCode    = buffer.data();

  Check(vkCreateShaderModule(m_device->Handle(), &info, nullptr, &m_module),
        "create shader module");
}
Shader::Shader(Shader&& other) noexcept {
  m_stage      = other.m_stage;
  m_module     = std::exchange(other.m_module, nullptr);
  m_entryPoint = std::move(other.m_entryPoint);
  m_name       = std::move(other.m_name);
}

Shader::~Shader() {
  vkDestroyShaderModule(m_device->Handle(), m_module, nullptr);
}

VkShaderStageFlagBits Shader::Stage() const {
  return m_stage;
}
const std::string& Shader::Name() const {
  return m_name;
}
const std::string& Shader::EntryPoint() const {
  return m_entryPoint;
}
const VkShaderModule Shader::Module() const {
  return m_module;
}
}  // namespace ezg::vk
