#include "main.hpp"
#include <log/log.hpp>
#include <common.hpp>
#include <time/udl.hpp>
#include <led/led.hpp>
#include <rf/rf.hpp>
#include <animation/animation.hpp>
#include <animation/spinner.hpp>
#include <stdbool.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define EVER ;;

using namespace hal::pin;
using namespace hal::led;
using namespace hal::mux;
using namespace app::animation;

constexpr static const wrapper::log::Module mod = wrapper::log::Module::MAIN;

void app_main() {
  static wrapper::log::Logger logger = wrapper::log::Logger::getInstance()
    .setThresholdLevel(ESP_LOG_DEBUG)
    .setActiveModules({
      wrapper::log::Module::MAIN,
      wrapper::log::Module::PIN,
      wrapper::log::Module::LED,
      wrapper::log::Module::RF,
      wrapper::log::Module::TASK,
    });
/*
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
  
  static const int blink_delay = 0.5_s;
  builtin_led.setBlinkDelay(blink_delay);
  logger.log(mod, ESP_LOG_INFO, "blink_delay = %d ms", blink_delay);
  builtin_led.getBlinkTask().start("BuiltinLedTask", 0, 4096);

  hal::rf::TxPwm tx(GPIO_NUM_18);
  tx.getTxTask().start("TxTask", 0, 8192);

  static const int max_duty = 10;
  static const int duty_increment = 2;
  static int current_duty = 0;
  for (EVER) {
    tx.sendDutyAsync((float)current_duty/10);
    current_duty += duty_increment;
    current_duty %= max_duty;

    logger.log(mod, ESP_LOG_DEBUG, "Main iteration");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  */

  auto led_config = (gpio_config_t) {
    .pin_bit_mask =
      1ULL << 25 |
      1ULL << 26 |
      1ULL << 32 |
      1ULL << 33,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };

  auto ledRingDemux = Demux{
      Pin(GPIO_NUM_25, led_config),
      Pin(GPIO_NUM_26, led_config),
      Pin(GPIO_NUM_32, led_config),
      Pin(GPIO_NUM_33, led_config),
    };

  auto spinnerFw = SpinnerForwardAnimation(ledRingDemux);
  auto animator = Animator((IAnimation*) &spinnerFw, 50_ms);
}