#include "led.hpp"
#include <stdbool.h>

using namespace hal::led;

[[noreturn]] void Led::blinkTask(const void *params) {
  while (true) {
    setState(hal::pin::State::In::TOGGLE);
    vTaskDelay(pdMS_TO_TICKS(blink_delay));
  }
}