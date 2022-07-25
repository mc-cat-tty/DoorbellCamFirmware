#pragma once
#include <time/udl.hpp>
#include <pin/pin.hpp>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace hal::led {
  class Led : hal::pin::Pin {
    private:
    int blink_delay = 0_s;

    public:
    using hal::pin::Pin::Pin;

    [[nodiscard]] constexpr int getBlinkDelay() const { return blink_delay; }
    void setBlinkDelay(int delay) { blink_delay = delay; }
  
    [[noreturn]] void blinkTask(const void *params);
  };
}