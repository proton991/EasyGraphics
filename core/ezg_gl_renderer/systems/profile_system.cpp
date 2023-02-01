#include "profile_system.hpp"

namespace ezg::system {
StopWatch::StopWatch() {
  m_initialisation_time = std::chrono::high_resolution_clock::now();

  m_last_time = std::chrono::high_resolution_clock::now();
}

float StopWatch::time_step() {
  const auto current_time = std::chrono::high_resolution_clock::now();

  auto time_duration =
      std::chrono::duration<float, std::chrono::seconds::period>(current_time - m_last_time)
          .count();

  m_last_time = current_time;

  return time_duration;
}

float StopWatch::time_step_since_initialisation() {
  auto current_time = std::chrono::high_resolution_clock::now();

  auto time_duration = std::chrono::duration<float, std::chrono::seconds::period>(
                           current_time - m_initialisation_time)
                           .count();

  return time_duration;
}
}  // namespace ezg::system
