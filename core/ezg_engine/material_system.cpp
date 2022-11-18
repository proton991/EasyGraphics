#include "material_system.hpp"

namespace ezg {
void MaterialSystem::Init() {}

Material* MaterialSystem::GetMaterial(const std::string& name) {
  auto it = m_materials.find(name);
  if (it != m_materials.end()) {
    return (*it).second;
  } else {
    return nullptr;
  }
}

void MaterialSystem::CreateMaterial(const std::string& name, VkPipeline pipeline,
                                    VkPipelineLayout pipelineLayout) {
  if (m_materials.find(name) != m_materials.end()) {
    return;
  } else {
    auto* material           = new Material();
    material->name           = name;
    material->pipeline       = pipeline;
    material->pipelineLayout = pipelineLayout;
    m_materials[name]        = material;
  }
}
}  // namespace ezg