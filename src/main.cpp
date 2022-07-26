#include "main.hpp"
#include <log/log.hpp>
#include <time/udl.hpp>
#include <led/led.hpp>
#include <stdbool.h>
#include "driver/gpio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define EVER ;;


namespace wrapper::log {
  enum class Module {
    MAIN,
    PIN,
    LED,
    DIM,
  };

  class MyLogger : public Logger {
    public:
    using Logger::Logger;

    const char* moduleToString(Module module) const {
      return module_to_str[static_cast<int>(module)];
    }

    private:
    const char *module_to_str[static_cast<int>(Module::DIM)] = {
      "main",
      "hal::pin",
      "hal::led",
    };
  };
}

constexpr static const wrapper::log::Module mod = wrapper::log::Module::MAIN;

void app_main() {
  wrapper::log::MyLogger logger(ESP_LOG_DEBUG);

  gpio_config_t builtin_led_config = {
    .pin_bit_mask = 1ULL << GPIO_NUM_2,
    .mode = GPIO_MODE_INPUT_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  hal::led::Led builtin_led(GPIO_NUM_2, builtin_led_config);
  if (!builtin_led.isOk())
    logger.log(mod, ESP_LOG_ERROR, "Error while building builtin_led");
  /// builtin_led.setBlinkDelay(0.5_s);
  // builtin_led.startBlink(5);

  for (EVER) {
    try {
      builtin_led.setState(hal::pin::State::In::HIGH);
      vTaskDelay(pdMS_TO_TICKS(500));
      builtin_led.setState(hal::pin::State::In::LOW);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    catch (const std::exception &e) {
      // ESP_LOGE(TAG, "%s", e.what());
    }
  }
}