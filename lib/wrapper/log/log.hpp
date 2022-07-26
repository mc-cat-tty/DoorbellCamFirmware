#pragma once
#include <stdint.h>
#include <stdarg.h>
#include "esp_log.h"

namespace wrapper::log {
  typedef void (*loggerCallback) (const char *, const char *, ...);

  class Logger {
    private:
    esp_log_level_t log_level;
    uint64_t log_active_modules;  // bit vector
    loggerCallback logger_callbacks[6] = {
      NULL,
      ESP_LOGE,
      ESP_LOGW,
      ESP_LOGI,
      ESP_LOGD,
      ESP_LOGV,
    };

    
    [[nodiscard]] constexpr bool isModuleActive(uint64_t module_mask) const {
      return (log_active_modules & module_mask) == module_mask;
    }

    public:
    Logger(esp_log_level_t level) : log_level(level) {
      esp_log_level_set("*", level);
    }

    enum class Module;
    
    virtual const char* moduleToString(int module) const = 0;

    [[nodiscard]] constexpr uint64_t moduleToInt(Module module) const {
      return 1ULL << static_cast<int>(module);
    }

    void log(Module module, esp_log_level_t level, const char *msg, ...) const;    
  };
}