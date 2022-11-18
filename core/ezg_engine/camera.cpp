#include "camera.hpp"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
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

void Camera::ProcessInputEvent(float velocity) {
  const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);
  if (m_dirty) {
    if (currentKeyStates[SDL_SCANCODE_UP] || currentKeyStates[SDL_SCANCODE_W]) {
      m_position += m_front * velocity;
    } else if (currentKeyStates[SDL_SCANCODE_DOWN] || currentKeyStates[SDL_SCANCODE_S]) {
      m_position -= m_front * velocity;
    } else if (currentKeyStates[SDL_SCANCODE_LEFT] || currentKeyStates[SDL_SCANCODE_A]) {
      m_position -= m_right * velocity;
    } else if (currentKeyStates[SDL_SCANCODE_RIGHT] || currentKeyStates[SDL_SCANCODE_D]) {
      m_position += m_right * velocity;
    } else if (currentKeyStates[SDL_SCANCODE_SPACE]) {
      m_position += m_worldUp * velocity;
    } else if (currentKeyStates[SDL_SCANCODE_LCTRL]) {
      m_position -= m_worldUp * velocity;
    }
    // process mouse movements
    int xPos, yPos;
    SDL_GetMouseState(&xPos, &yPos);
    if (m_firstMouse) {
      m_mousePosX  = xPos;
      m_mousePosY  = yPos;
      m_firstMouse = false;
    }
    const auto xOffset = (m_mousePosX - xPos) * m_sensitivity;
    const auto yOffset = -1 * (m_mousePosY - yPos) * m_sensitivity;

    m_mousePosX = xPos;
    m_mousePosY = yPos;

    m_yaw += static_cast<float>(xOffset);
    m_pitch += static_cast<float>(yOffset);
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
  }
}

void Camera::Update(float deltaTime) {
  const float velocity = m_speed * deltaTime;
  ProcessInputEvent(velocity);
  UpdateBaseVectors();
}

glm::mat4 Camera::GetViewMatrix() {
  return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::GetProjectionMatrix() {
  return m_projMatrix;
}
}  // namespace ezg