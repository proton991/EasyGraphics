#include "camera.hpp"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "input_controller.hpp"
namespace ezg {
Camera::Camera(glm::vec3 eye, glm::vec3 target, float fov, float aspect, float near, float far)
    : m_position{eye}, m_aspect{aspect}, m_fovY{fov}, m_near{near}, m_far{far} {
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
  m_right = glm::normalize(glm::cross(m_front, m_worldUp));
  m_up    = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::SetProjectionMatrix() {
  assert(glm::abs(m_aspect - std::numeric_limits<float>::epsilon()) > 0.f);
  m_projMatrix = glm::perspective(glm::radians(m_fovY), m_aspect, m_near, m_far);
  // compared to opengl's coordinate system, vulkan has y-axis inverted
  m_projMatrix[1][1] *= -1;
}

void Camera::UpdatePosition(float velocity) {
  if (m_dirty) {
    if (CameraController::Get().MoveForward()) {
      m_position += m_front * velocity;
    }
    if (CameraController::Get().MoveBackWard()) {
      m_position -= m_front * velocity;
    }
    if (CameraController::Get().MoveLeft()) {
      m_position -= m_right * velocity;
    }
    if (CameraController::Get().MoveRight()) {
      m_position += m_right * velocity;
    }
    if (CameraController::Get().MoveUp()) {
      m_position += m_worldUp * velocity;
    }
    if (CameraController::Get().MoveDown()) {
      m_position -= m_worldUp * velocity;
    }
  }
}

void Camera::UpdateView() {
  auto mouseMove = CameraController::Get().GetMouseMove();
  m_yaw += mouseMove.first * m_sensitivity;
  m_pitch += mouseMove.second * m_sensitivity;
  m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
}

void Camera::Update(float deltaTime) {
  CameraController::Get().Update();
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
}  // namespace ezg