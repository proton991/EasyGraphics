#ifndef TIMER_HPP
#define TIMER_HPP
#include <chrono>
namespace ezg::util {
class FrameTimer {
  // The time point of the last render call.
  std::chrono::time_point<std::chrono::high_resolution_clock> m_last_time;

  // The time point of initialisation.
  std::chrono::time_point<std::chrono::high_resolution_clock> m_initialisation_time;

public:
  FrameTimer();

  /// @brief Return a scaling factor which corresponds to the
  /// time which has passed since last render call and now.
  [[nodiscard]] float TimeStep();

  /// @brief Return a scaling factor which corresponds to the
  /// time which has passed since initialisation and now.
  [[nodiscard]] float TimeStepSinceInitialisation();
};
}

#endif  //TIMER_HPP
