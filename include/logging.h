#ifndef REPLIKON_LOGGING_H
#define REPLIKON_LOGGING_H

#include "utils.h"
#include <string>

/**
    0 - disabled logging
    1 - Error
    2 - Warnings
    3 - Info
    4 - Debug
*/
#define REPLIKON_LOG_LEVEL 4

namespace replikon {

constexpr const char *prefix(int log_level) {
  switch (log_level) {
  case 1:
    return "[ERROR]";
  case 2:
    return "[WARN]";
  case 3:
    return "[INFO]";
  case 4:
    return "[DEBUG]";
  default:
    REPLIKON_UNREACHABLE;
  }
}

void setLogFile(FILE *file);
FILE *getLogFile();

template <int LOG_LEVEL, typename... Args>
void LOG(const char *pattern, Args... args) { // NOLINT(readability-identifier-naming)
  static_assert(LOG_LEVEL >= 1 && LOG_LEVEL <= 4);
  if constexpr (LOG_LEVEL <= REPLIKON_LOG_LEVEL) {
    REPLIKON_ASSERT(getLogFile() != nullptr);
    auto prefixed_pattern =
        std::string(prefix(LOG_LEVEL)) + " " + pattern + "\n";
    if constexpr (sizeof...(Args) == 0) {
      fprintf(getLogFile(), "%s", prefixed_pattern.c_str());
    } else {
      fprintf(getLogFile(), prefixed_pattern.c_str(), args...);
    }
  }
}

template <typename... Args> void LOGE(const char *pattern, Args... args) { // NOLINT(readability-identifier-naming)
  LOG<1, Args...>(pattern, std::forward<Args>(args)...);
}

template <typename... Args> void LOGW(const char *pattern, Args... args) { // NOLINT(readability-identifier-naming)
  LOG<2, Args...>(pattern, std::forward<Args>(args)...);
}

template <typename... Args> void LOGI(const char *pattern, Args... args) { // NOLINT(readability-identifier-naming)
  LOG<3, Args...>(pattern, std::forward<Args>(args)...);
}

template <typename... Args> void LOGD(const char *pattern, Args... args) { // NOLINT(readability-identifier-naming)
  LOG<4, Args...>(pattern, std::forward<Args>(args)...);
}

} // namespace replikon
#endif // REPLIKON_LOGGING_H