#ifndef EASYGRAPHICS_CAMERA_SYSTEM_HPP
#define EASYGRAPHICS_CAMERA_SYSTEM_HPP
#include <glm/glm.hpp>

namespace ezg::system {

class Camera {
public:
  Camera(glm::vec3 eye, glm::vec3 target, float fov, float aspect, float near, float far);
  glm::mat4 GetViewMatrix();
  glm::mat4 GetProjectionMatrix();

  void Update(float deltaTime);

private:
  void SetProjectionMatrix();
  void UpdateBaseVectors();
  void UpdatePosition(float velocity);
  void UpdateView();
  // camera attributes
  glm::vec3 m_position{0.0f, 0.0f, 0.0f};
  glm::vec3 m_front{0.0f, 0.0f, -1.0f};
  glm::vec3 m_up{0.0f, 1.0f, 0.0f};
  glm::vec3 m_right;
  const glm::vec3 m_world_up{0.0f, 1.0f, 0.0f};

  float m_aspect;
  float m_fov;

  float m_yaw;
  float m_pitch;

  float m_near{1.0f};
  float m_far{100.0f};

  glm::mat4 m_projMatrix{1.f};
  glm::mat4 m_viewMatrix{1.f};

  float m_speed{10.f};
  const float m_sensitivity{0.5f};

  // should we update camera
  bool m_dirty{true};
};

}  // namespace ezg::system
#endif  //EASYGRAPHICS_CAMERA_SYSTEM_HPP
