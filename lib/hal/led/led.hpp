#pragma once
#include <pin/pin.hpp>

namespace hal::led {
  class Led : hal::pin::Pin {
    private:
    int blink_delay = 0;
    public:
    using hal::pin::Pin::Pin;

    [[noreturn]] void blinkTask(const void *params);
  };
}