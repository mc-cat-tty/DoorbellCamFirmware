#include "common.hpp"
#include <vector>

namespace wrapper::log {
  std::vector<const char *> module_to_str = {
    "main",
    "hal::pin",
    "hal::led",
    "hal::rf",
    "wrapper::task",
    "app::animation",
  };
}