#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <vector>
#include <utility>
#include "esp_log.h"

namespace wrapper::log {
  enum class Module;
  extern std::vector<const char*> module_to_str;
  
  class Logger {  // singleton
    private:
    esp_log_level_t log_level = ESP_LOG_WARN;
    uint64_t log_active_modules;  // bit vector

    [[nodiscard]] inline constexpr bool isModuleActive(uint64_t module_mask) const {
      return (log_active_modules & module_mask) == module_mask;
    }

    Logger() {}

    public:
    [[nodiscard]] inline static Logger& getInstance() {
      static Logger instance;
      return instance;
    }

    inline Logger& setThresholdLevel(esp_log_level_t level) {
      log_level = level;
      esp_log_level_set("*", level);
      return *this;
    }

    inline Logger& setActiveModules(std::vector<Module> modules) {
      for (const Module &mod : modules) {
        log_active_modules |= moduleToInt(mod);
      }
      return *this;
    }
    
    [[nodiscard]] inline const char* moduleToString(Module module) const {
      return module_to_str[static_cast<int>(module)];
    }

    [[nodiscard]] inline constexpr uint64_t moduleToInt(Module module) const {
      return 1ULL << static_cast<int>(module);
    }

    template <typename... Types>
    void log(Module module, esp_log_level_t level, const char *format, Types&&... args) const {
      if (!isModuleActive(moduleToInt(module)))
        return;
      char msg[128];
      sprintf(msg, format, std::forward<Types>(args)...);
      ESP_LOG_LEVEL(level, moduleToString(module), "%s", msg);
    }   
  };
}