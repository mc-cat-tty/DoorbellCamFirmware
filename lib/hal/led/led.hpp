#pragma once
#include <string>
#include <functional>
#include <time/udl.hpp>
#include <pin/pin.hpp>
#include <task/task.hpp>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace hal::led {
  class Led : public hal::pin::Pin {
    private:
    int blink_delay = 0_s;
    wrapper::task::Task<hal::led::Led> blink_task;

    [[noreturn]] void blinkTask();

    public:
    constexpr Led(gpio_num_t pin_num, const gpio_config_t& pin_config)
      : Pin(pin_num, pin_config),
      blink_task(&hal::led::Led::blinkTask, this) {}

    [[nodiscard]] inline constexpr int getBlinkDelay() const { return blink_delay; }
    inline void setBlinkDelay(int delay) { blink_delay = delay; }

    [[nodiscard]] inline constexpr wrapper::task::Task<hal::led::Led> getBlinkTask()
      const { return blink_task; }
  };
}