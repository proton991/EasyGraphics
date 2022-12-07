#ifndef EASYGRAPHICS_INPUT_CONTROLLER_HPP
#define EASYGRAPHICS_INPUT_CONTROLLER_HPP

#include <SDL2/SDL_events.h>
#include <array>

#define NUM_DIRECTIONS 8
namespace ezg {

enum Direction { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

class CameraController {
public:
  static CameraController& Get() {
    static CameraController instance;
    return instance;
  }

  void Update();

  [[nodiscard]] bool MoveForward() const;
  [[nodiscard]] bool MoveBackWard() const;
  [[nodiscard]] bool MoveUp() const;
  [[nodiscard]] bool MoveDown() const;
  [[nodiscard]] bool MoveLeft() const;
  [[nodiscard]] bool MoveRight() const;

  [[nodiscard]] std::pair<float, float> GetMouseMove() const;

private:
  CameraController();

  // mouse positions
  bool m_firstMouse{true};

  struct {
    int previousX{};
    int previousY{};
    int currentX{};
    int currentY{};
  } m_mousePos;

  std::array<bool, NUM_DIRECTIONS> m_actions{false};
};
}  // namespace ezg
#endif  //EASYGRAPHICS_INPUT_CONTROLLER_HPP
