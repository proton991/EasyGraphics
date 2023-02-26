#include "scene.hpp"
#include "managers/resource_manager.hpp"
#include <spdlog/spdlog.h>

namespace ezg::gl {
BaseScene::BaseScene(std::string_view name) : m_name(name) {
  spdlog::info("Setting up scene: {}", m_name);
}

void BaseScene::add_model(const Ref<Model>& model) {
  m_models.push_back(model);
}

void BaseScene::add_model(const std::string& model_name) {
  m_models.push_back(ResourceManager::GetInstance().get_model(model_name));
}

AABB BaseScene::get_aabb() const {
  AABB aabb = m_models[0]->get_aabb();
  for (int i = 1; i < m_models.size(); i++) {
    aabb.bbx_max = glm::max(m_models[i]->get_aabb().bbx_max, aabb.bbx_max);
    aabb.bbx_min = glm::max(m_models[i]->get_aabb().bbx_min, aabb.bbx_min);
  }
  return aabb;
}
}  // namespace ezg::gl