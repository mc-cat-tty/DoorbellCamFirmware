#pragma once
#include <array>

namespace wrapper::log {
  enum class Module {
    MAIN,
    PIN,
    LED,
    RF,
    TASK,
  };

  constexpr std::array<const char *, 5> module_to_str = {{
    "main",
    "hal::pin",
    "hal::led",
    "hal::rf",
    "wrapper::task",
  }};
}