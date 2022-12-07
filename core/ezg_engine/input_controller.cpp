#include "input_controller.hpp"
#include <iostream>
namespace ezg {
CameraController::CameraController() {
  std::fill(m_actions.begin(), m_actions.end(), false);
}
void CameraController::Update() {
  std::fill(m_actions.begin(), m_actions.end(), false);
  const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);
  if (currentKeyStates[SDL_SCANCODE_UP] || currentKeyStates[SDL_SCANCODE_W]) {
    m_actions[Direction::FORWARD] = true;
  }
  if (currentKeyStates[SDL_SCANCODE_DOWN] || currentKeyStates[SDL_SCANCODE_S]) {
    m_actions[Direction::BACKWARD] = true;
  }
  if (currentKeyStates[SDL_SCANCODE_LEFT] || currentKeyStates[SDL_SCANCODE_A]) {
    m_actions[Direction::LEFT] = true;
  }
  if (currentKeyStates[SDL_SCANCODE_RIGHT] || currentKeyStates[SDL_SCANCODE_D]) {
    m_actions[Direction::RIGHT] = true;
  }
  if (currentKeyStates[SDL_SCANCODE_SPACE]) {
    m_actions[Direction::UP] = true;
  }
  if (currentKeyStates[SDL_SCANCODE_LCTRL]) {
    m_actions[Direction::DOWN] = true;
  }
  // process mouse movements
  int xPos, yPos;
  SDL_GetMouseState(&xPos, &yPos);
  if (m_firstMouse) {
    m_mousePos.previousX = xPos;
    m_mousePos.previousY = yPos;
    m_mousePos.currentX  = xPos;
    m_mousePos.currentY  = yPos;
    m_firstMouse         = false;
  } else {
    m_mousePos.previousX = m_mousePos.currentX;
    m_mousePos.previousY = m_mousePos.currentY;
    m_mousePos.currentX  = xPos;
    m_mousePos.currentY  = yPos;
  }
}

std::pair<float, float> CameraController::GetMouseMove() const {
  const auto xOffset = static_cast<float>(m_mousePos.previousX - m_mousePos.currentX);
  const auto yOffset = static_cast<float>(m_mousePos.currentY - m_mousePos.previousY);
  std::pair<float, float> delta{xOffset, yOffset};
  return delta;
}

bool CameraController::MoveForward() const {
  return m_actions[Direction::FORWARD];
}

bool CameraController::MoveBackWard() const {
  return m_actions[Direction::BACKWARD];
}

bool CameraController::MoveUp() const {
  return m_actions[Direction::UP];
}

bool CameraController::MoveDown() const {
  return m_actions[Direction::DOWN];
}

bool CameraController::MoveLeft() const {
  return m_actions[Direction::LEFT];
}

bool CameraController::MoveRight() const {
  return m_actions[Direction::RIGHT];
}

}  // namespace ezg