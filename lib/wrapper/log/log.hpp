#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <utility>
#include "esp_log.h"

namespace wrapper::log {
  enum class Module;
  
  class Logger {
    private:
    esp_log_level_t log_level;
    uint64_t log_active_modules;  // bit vector


    [[nodiscard]] constexpr bool isModuleActive(uint64_t module_mask) const {
      return (log_active_modules & module_mask) == module_mask;
    }

    public:
    Logger(esp_log_level_t level) : log_level(level) {
      esp_log_level_set("*", level);
    }
    
    virtual const char* moduleToString(Module module) const = 0;

    [[nodiscard]] constexpr uint64_t moduleToInt(Module module) const {
      return 1ULL << static_cast<int>(module);
    }

    template <typename... Types>
    void log(Module module, esp_log_level_t level, const char *format, Types&&... args) const;    
  };
}