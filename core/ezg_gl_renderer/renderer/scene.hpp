#ifndef SCENE_HPP
#define SCENE_HPP
#include "assets/model.hpp"
#include "base.hpp"

namespace ezg::gl {
class BaseScene;

class SceneBuilder {
public:
  template <typename T>
  static Ref<BaseScene> Create(const std::string& name) {
    return CreateRef<T>(name);
  }
};
class BaseScene {
public:
  friend class BasicRenderer;
  explicit BaseScene(std::string_view name);
  // disable copying
  BaseScene& operator=(const BaseScene&) = delete;
  BaseScene(const BaseScene&)            = delete;

  virtual void init() {};
  void add_model(const std::string& model_name);
  void add_model(const Ref<Model>& model);

  void update(float time = 0.0f);

  [[nodiscard]] AABB get_aabb() const;

  const auto& get_light_pos() const { return m_light_position; }
  const auto& get_light_dir() const { return m_light_dir; }
  const auto& get_light_intensity() const { return m_light_intensity; }
  void switch_camera_light();
  auto light_from_camera() const { return m_camera_light_on; }

protected:
  std::string m_name;
  std::vector<Ref<Model>> m_models;
  // light comes from camera
  glm::vec3 m_light_position{10.f, 10.f, 10.f};
  glm::vec3 m_light_dir{0.0f, 0.0f, -1.0f};
  glm::vec3 m_light_intensity{5.0f};
  bool m_camera_light_on{true};
};
}  // namespace ezg::gl

#endif  //SCENE_HPP
