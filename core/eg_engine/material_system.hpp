#ifndef MATERIAL_SYSTEM_HPP
#define MATERIAL_SYSTEM_HPP
#include <vulkan/vulkan.h>
#include <string>
#include <unordered_map>

namespace ege {
struct MaterialInfo {};
struct Material {
  std::string name;
  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
  VkDescriptorSet textureSet{VK_NULL_HANDLE};
};
class MaterialSystem {
public:
  void Init();

  Material* GetMaterial(const std::string& name);

  void CreateMaterial(const std::string& name, VkPipeline pipeline,
                      VkPipelineLayout pipelineLayout);

private:
  std::unordered_map<std::string, Material*> m_materials;
};
}  // namespace ege
#endif  //MATERIAL_SYSTEM_HPP
