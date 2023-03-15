#include "scene.hpp"
#include "log.hpp"
#include "managers/resource_manager.hpp"
#include "systems/input_system.hpp"

namespace ezg::gl {
BaseScene::BaseScene(std::string_view name) : m_name(name) {
  spdlog::info("Loading scene: {}", m_name);
}

void BaseScene::add_model(const Ref<Model>& model) {
  m_models.push_back(model);
}

void BaseScene::add_model(const std::string& model_path) {
  m_models.push_back(ResourceManager::GetInstance().load_gltf_model(model_path));
}

void BaseScene::update(float time) {
  // turn on/off light
  if (system::KeyboardMouseInput::GetInstance().was_key_pressed_once(GLFW_KEY_L)) {
    switch_light();
  }
  if (time != 0.0f) {
    float rotation_angle = time * 0.5f;
    for (auto& model : m_models) {
      model->rotate(rotation_angle);
    }
  }
}

AABB BaseScene::get_aabb() const {
  AABB aabb = m_models[0]->get_aabb();
  for (int i = 1; i < m_models.size(); i++) {
    aabb.bbx_max = glm::max(m_models[i]->get_aabb().bbx_max, aabb.bbx_max);
    aabb.bbx_min = glm::max(m_models[i]->get_aabb().bbx_min, aabb.bbx_min);
  }
  return aabb;
}

void BaseScene::switch_light() {
  m_light_on = !m_light_on;
  if (!m_light_on) {
    m_light_intensity = glm::vec3(0.0f);
    spdlog::info("light off");
  } else {
    m_light_intensity = glm::vec3(5.0f);
    spdlog::info("light on");
  }
}
}  // namespace ezg::gl