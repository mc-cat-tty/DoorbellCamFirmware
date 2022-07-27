#include "main.hpp"
#include <log/log.hpp>
#include <common.hpp>
#include <time/udl.hpp>
#include <led/led.hpp>
#include <stdbool.h>
#include "driver/gpio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define EVER ;;

constexpr static const wrapper::log::Module mod = wrapper::log::Module::MAIN;

void app_main() {
  wrapper::log::Logger logger = wrapper::log::Logger::getInstance()
    .setThresholdLevel(ESP_LOG_DEBUG)
    .setActiveModules({
      wrapper::log::Module::MAIN,
      wrapper::log::Module::PIN,
      wrapper::log::Module::LED,
    });

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
      logger.log(mod, ESP_LOG_ERROR, e.what());
    }
  }
}