#ifndef SCENE_HPP
#define SCENE_HPP
#include "assets/model.hpp"
namespace ezg::gl {
class BaseScene;
using BaseScenePtr = std::shared_ptr<BaseScene>;
class BaseScene {
public:
  friend class BasicRenderer;
  explicit BaseScene(std::string_view name);
  // disable copying
  BaseScene& operator=(const BaseScene&) = delete;
  BaseScene(const BaseScene&)            = delete;

  virtual void init() = 0;
  void add_model(const std::string& model_name);
  void add_model(const ModelPtr & model);

  [[nodiscard]] AABB get_aabb() const;

  const auto& get_light_pos() const { return m_light_position; }
  const auto& get_light_intensity() const { return m_light_intensity; }
private:
  std::string m_name;
  std::vector<ModelPtr> m_models;
  // light comes from camera
  glm::vec3 m_light_position{0.f, 0.f, 1.f};
  glm::vec3 m_light_intensity{5.0f};
};
}  // namespace ezg::gl

#endif  //SCENE_HPP
