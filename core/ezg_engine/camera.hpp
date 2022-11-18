#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <SDL2/SDL_events.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace ezg {
class Camera {
public:
  Camera(glm::vec3 eye, glm::vec3 target, float fov, float aspect, float near, float far);
  glm::mat4 GetViewMatrix();
  glm::mat4 GetProjectionMatrix();

  void ProcessInputEvent(float velocity);
  void Update(float deltaTime);

private:
  void SetProjectionMatrix();
  void UpdateBaseVectors();

  // camera attributes
  glm::vec3 m_position{};
  glm::vec3 m_front;
  glm::vec3 m_up;
  glm::vec3 m_right;
  const glm::vec3 m_worldUp{0.f, -1.f, 0.f};

  float m_aspect;
  float m_fovY;

  float m_yaw;
  float m_pitch;

  float m_near;
  float m_far;

  glm::mat4 m_projMatrix{1.f};
  glm::mat4 m_viewMatrix{1.f};

  float m_speed{10.f};
  const float m_sensitivity{0.5f};

  // should we update camera
  bool m_dirty{true};
  // mouse positions
  bool m_firstMouse{true};
  double m_mousePosX{};
  double m_mousePosY{};
};
}  // namespace ezg

#endif  //CAMERA_HPP
