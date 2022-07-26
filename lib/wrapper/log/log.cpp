#include "log.hpp"
#include <stdio.h>

using namespace wrapper::log;

template <typename... Types>
void Logger::log(Module module, esp_log_level_t level, const char *format, Types&&... args) const {
  if (!isModuleActive(moduleToInt(module)))
    return;
  char msg[128];
  sprintf(msg, format, std::forward<Types>(args)...);
  ESP_LOG_LEVEL(level, moduleToString(module), "%s", msg);
}