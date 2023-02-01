#ifndef EASYGRAPHICS_PROFILE_SYSTEM_HPP
#define EASYGRAPHICS_PROFILE_SYSTEM_HPP

#include <chrono>

namespace ezg::system {
class StopWatch {
  // The time point of the last render call.
  std::chrono::time_point<std::chrono::high_resolution_clock> m_last_time;

  // The time point of initialisation.
  std::chrono::time_point<std::chrono::high_resolution_clock> m_initialisation_time;

public:
  StopWatch();

  /// @brief Return a scaling factor which corresponds to the
  /// time which has passed since last render call and now.
  [[nodiscard]] float time_step();

  /// @brief Return a scaling factor which corresponds to the
  /// time which has passed since initialisation and now.
  [[nodiscard]] float time_step_since_initialisation();
};
}  // namespace ezg::system
#endif  //EASYGRAPHICS_PROFILE_SYSTEM_HPP
