#include "camera_system.hpp"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "systems/input_system.hpp"
namespace ezg::system {
Camera Camera::CreateDefault() {
  glm::vec3 eye    = {0.0f, 0.0f, -5.0f};
  glm::vec3 target = {0.0f, 0.0f, 0.0f};
  float fov        = 70.0f;
  float near       = 1.0f;
  float far        = 100.0f;

  return {eye, target, fov, 1.0f, near, far};
}

Camera Camera::CreateBasedOnBBox(const glm::vec3& bbox_min, const glm::vec3& bbox_max) {
  const auto diag   = bbox_max - bbox_min;
  auto max_distance = glm::length(diag);
  float near        = 0.001f * max_distance;
  float far         = 5.f * max_distance;
  float fov         = 70.0f;
  const auto center = 0.5f * (bbox_max + bbox_min);
  const auto up     = glm::vec3(0, 1, 0);
  const auto eye    = diag.z > 0 ? center + diag : center + 2.f * glm::cross(diag, up);
  return {eye, center, fov, 1.0f, near, far};
}
Camera::Camera(glm::vec3 eye, glm::vec3 target, float fov, float aspect, float near, float far)
    : m_position{eye}, m_aspect{aspect}, m_fov{fov}, m_near{near}, m_far{far} {
  glm::vec3 direction = glm::normalize(target - m_position);

  m_pitch = glm::degrees(asin(direction.y));
  m_yaw   = glm::degrees(atan2(direction.z, direction.x));

  UpdateBaseVectors();
  SetProjectionMatrix();
}

void Camera::UpdateBaseVectors() {
  m_front.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
  m_front.y = glm::sin(glm::radians(m_pitch));
  m_front.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
  // Calculate the new Front vector

  m_front = glm::normalize(m_front);
  m_right = glm::normalize(glm::cross(m_front, m_world_up));
  m_up    = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::SetProjectionMatrix() {
  assert(glm::abs(m_aspect - std::numeric_limits<float>::epsilon()) > 0.f);
  m_projMatrix = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
}

void Camera::UpdatePosition(float velocity) {
  if (m_dirty) {
    if (KeyboardMouseInput::GetInstance().was_key_pressed_once(GLFW_KEY_UP)) {
      m_speed *= 2;
    }
    if (KeyboardMouseInput::GetInstance().was_key_pressed_once(GLFW_KEY_DOWN)) {
      m_speed /= 2;
    }
    if (KeyboardMouseInput::GetInstance().is_key_pressed(GLFW_KEY_SPACE)) {
      // reset position
      m_position = {0.0f, 0.0f, -5.0f};
    }
    if (KeyboardMouseInput::GetInstance().is_key_pressed(GLFW_KEY_W)) {
      m_position += m_front * velocity;
    }
    if (KeyboardMouseInput::GetInstance().is_key_pressed(GLFW_KEY_S)) {
      m_position -= m_front * velocity;
    }
    if (KeyboardMouseInput::GetInstance().is_key_pressed(GLFW_KEY_A)) {
      m_position -= m_right * velocity;
    }
    if (KeyboardMouseInput::GetInstance().is_key_pressed(GLFW_KEY_D)) {
      m_position += m_right * velocity;
    }
    if (KeyboardMouseInput::GetInstance().is_key_pressed(GLFW_KEY_LEFT_SHIFT)) {
      m_position += m_world_up * velocity;
    }
    if (KeyboardMouseInput::GetInstance().is_key_pressed(GLFW_KEY_LEFT_CONTROL)) {
      m_position -= m_world_up * velocity;
    }
  }
}

void Camera::UpdateView() {
  auto delta = KeyboardMouseInput::GetInstance().calculate_cursor_position_delta();
  m_yaw += delta[0] * m_sensitivity;
  m_pitch += delta[1] * m_sensitivity;
  m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
}

void Camera::Update(float deltaTime) {
  const float velocity = m_speed * deltaTime;
  UpdatePosition(velocity);
  UpdateView();
  UpdateBaseVectors();
}

glm::mat4 Camera::GetViewMatrix() {
  return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::GetProjectionMatrix() {
  return m_projMatrix;
}
}  // namespace ezg::system