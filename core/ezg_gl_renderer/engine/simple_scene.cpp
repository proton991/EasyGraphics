#include "simple_scene.hpp"
#include <glm/gtx/string_cast.hpp>
#include "managers/resource_manager.hpp"
#include "assets/skybox.hpp"

namespace ezg::gl {
void SimpleScene::init() {
  // setup skybox
  m_skybox = Skybox::Create("../resources/textures/hdri/barcelona.hdr", 2048);
  add_model(ModelPaths[0]);
  load_floor();
}

void SimpleScene::load_new_model(uint32_t index) {
  m_models.clear();
  add_model(ModelPaths[index]);
}

void SimpleScene::load_floor() {
  m_floor = ResourceManager::GetInstance().load_gltf_model(FloorPath);

  const auto aabb         = get_aabb();
  const auto scene_size   = glm::length(aabb.diag);
  const auto floor_size   = glm::length(m_floor->get_aabb().diag);
  const auto scale_factor = 3 * scene_size / floor_size;
  m_floor->scale(scale_factor);
  m_floor->translate(glm::vec3(0.0, aabb.bbx_min.y * 1.01, 0.0));
  ResourceManager::GetInstance().delete_model(m_floor->get_name());
}
}  // namespace ezg::gl
