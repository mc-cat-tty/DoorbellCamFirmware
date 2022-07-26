#include "led.hpp"

#define EVER ;;

using namespace hal::led;

[[noreturn]] void Led::__blinkTask() {
  for (EVER) {
    setState(hal::pin::State::In::TOGGLE);
    vTaskDelay(pdMS_TO_TICKS(blink_delay));
  }
}

[[noreturn]] void Led::blinkTask(void *params) {
  static_cast<hal::led::Led *>(params)->__blinkTask();
}