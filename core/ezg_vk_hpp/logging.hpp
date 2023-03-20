#ifndef LOGGING_HPP
#define LOGGING_HPP
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#define SPDLOG_DEFAULT_PATTERN "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v"
// A thin wrapper of spdlog
namespace ezg::logger {
inline void SetDefaultPattern() {
  spdlog::set_pattern(SPDLOG_DEFAULT_PATTERN);
}

inline void SetListPattern() {
  spdlog::set_pattern("\t\t\t\t %v");
}
template <typename T>
inline void info(const T& msg) {
  spdlog::info(msg);
}

template <typename... Args>
inline void info(spdlog::format_string_t<Args...> fmt, Args&&... args) {
  spdlog::info(fmt, std::forward<Args>(args)...);
}

template <typename T>
inline void warn(const T& msg) {
  spdlog::warn(msg);
}

template <typename... Args>
inline void warn(spdlog::format_string_t<Args...> fmt, Args&&... args) {
  spdlog::warn(fmt, std::forward<Args>(args)...);
}

template <typename T>
inline void error(const T& msg) {
  spdlog::error(msg);
}

template <typename... Args>
inline void error(spdlog::format_string_t<Args...> fmt, Args&&... args) {
  spdlog::error(fmt, std::forward<Args>(args)...);
}

template <typename T>
inline void critical(const T& msg) {
  spdlog::critical(msg);
}

template <typename... Args>
inline void critical(spdlog::format_string_t<Args...> fmt, Args&&... args) {
  spdlog::critical(fmt, std::forward<Args>(args)...);
}

template <typename T>
inline void LogList(const std::vector<T>& l) {
  logger::SetListPattern();
  for (const auto& item : l) {
    spdlog::info("{}", item);
  }
  logger::SetDefaultPattern();
}
}  // namespace ezg::logger

#endif  //LOGGING_HPP
