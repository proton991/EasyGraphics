#ifndef SCENE_HPP
#define SCENE_HPP
#include "assets/model.hpp"
#include "ezg_gl_renderer/assets/skybox.hpp"
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
  virtual void load_floor() {};
  virtual void load_light_model() {};
  void add_model(const std::string& model_name);
  void add_model(const Ref<Model>& model);
  virtual void load_new_model(uint32_t index) = 0;

  void update(float time = 0.0f);

  [[nodiscard]] AABB get_aabb() const;

  const auto get_light_pos() const { return m_light_model->get_aabb().get_center(); }
  const auto& get_light_dir() const { return m_light_dir; }
  const auto& get_light_intensity() const { return m_light_intensity; }
  void switch_light();
  auto has_skybox() const { return m_skybox != nullptr;}

  virtual int get_num_models() = 0;
  virtual const char** get_model_data() = 0;

protected:
  std::string m_name;
  std::vector<Ref<Model>> m_models;
  Ref<Model> m_floor;
  Ref<Model> m_light_model;
  Ref<Skybox> m_skybox;
  glm::vec3 m_light_dir{0.0f, 0.0f, -1.0f};
  glm::vec3 m_light_intensity{1.0f};
  bool m_light_on{true};
};
}  // namespace ezg::gl

#endif  //SCENE_HPP
