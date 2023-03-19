#ifndef LOGGING_HPP
#define LOGGING_HPP
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#define SPDLOG_DEFAULT_PATTERN "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v"

namespace ezg::logger {
inline void SetDefaultPattern() {
  spdlog::set_pattern(SPDLOG_DEFAULT_PATTERN);
}
}  // namespace ezg::logger

#endif  //LOGGING_HPP
