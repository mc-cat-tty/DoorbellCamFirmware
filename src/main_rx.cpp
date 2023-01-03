#ifdef RECEIVER

#include "main.hpp"
#include <log/log.hpp>
#include <common.hpp>
#include <time/udl.hpp>
#include <led/led.hpp>
#include <rf/rf.hpp>
#include <animation/animation.hpp>
#include <animation/spinner.hpp>
#include <stdbool.h>
#include <math.h>
#include <driver/gpio.h>
#include <driver/mcpwm.h>
#include <driver/rmt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define EVER ;;

using namespace hal::pin;
using namespace hal::led;
using namespace hal::mux;
using namespace hal::rf;
using namespace wrapper::animation;

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
      wrapper::log::Module::ANIMATION,
    });

  auto rx = RxPwm(GPIO_NUM_23);
  rx.getRxTask().start(
    "rxTask",
    configMAX_PRIORITIES - 1,
    4096
  );

  auto rxSequence = RxSequence(
    rx,
    {1, 3, 5}
  );

  const auto ledsConfig = (gpio_config_t) {
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
    Led(GPIO_NUM_33, ledsConfig),
    Led(GPIO_NUM_25, ledsConfig),
    Led(GPIO_NUM_32, ledsConfig),
    Led(GPIO_NUM_26, ledsConfig),
  };

  auto spinnerFw = SpinnerForwardAnimation(ledRingDemux);
  auto animator = Animator(spinnerFw, 200_ms);
  logger.log(mod, ESP_LOG_DEBUG, "First animation run");

  for (EVER) {
    if (rxSequence.rcvdSequenceAsync()) {
      logger.log(mod, ESP_LOG_INFO, "Sequence matched");
      if (!animator.isRunning()) {
        spinnerFw = SpinnerForwardAnimation(ledRingDemux);
        animator = Animator(spinnerFw, 200_ms);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(1_s));
  }
}

#endif  // RECEIVER