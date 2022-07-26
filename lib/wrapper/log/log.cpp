#include "log.hpp"

using namespace wrapper::log;

void Logger::log(Module module, esp_log_level_t level, const char *msg, ...) const {
  if (isModuleActive(moduleToInt(module))) {
    va_list args;
    va_start(args, msg);
    logger_callbacks[level](moduleToString(module), msg, args);
    va_end(args);
  }
}