#include <cstdint>
#include <initializer_list>
#include <esp_log.h>

namespace wrapper::log {
  struct LoggerOptions {
    esp_log_level_t log_level;
    uint64_t log_active_modules;

    static constexpr uint64_t initModules(std::initializer_list<Module> modules) {
      auto res = 0UL;
      for (const auto& m : modules) {
        res |= (1 << int(m));
      }

      return res;
    }
  };
}