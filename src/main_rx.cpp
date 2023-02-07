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

constexpr static const uint8_t INIT_ITERATIONS = 5;

constexpr static const auto mod = wrapper::log::Module::MAIN;

using namespace hal::pin;
using namespace hal::led;
using namespace hal::mux;
using namespace hal::rf;
using namespace wrapper::animation;

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


  PwmData data;
  float upCountAvg = 0;
  for (int i=0; i<INIT_ITERATIONS;) {
    vTaskDelay(pdMS_TO_TICKS(1_s));

    if (rx.getPwmDataAsync(data)) {
      upCountAvg += data.upCount;
      i++;
    }
  }

  upCountAvg /= INIT_ITERATIONS;

  logger.log(mod, ESP_LOG_INFO, "upCountAvg: %d", (uint64_t) upCountAvg);

  auto rxSequence = RxDeltas(
    rx,
    {+0.5},
    0.45,
    upCountAvg + 1
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

  logger.log(mod, ESP_LOG_DEBUG, "First animation run");

  for (EVER) {
    if (rxSequence.rcvdDeltasAsync()) {
      logger.log(mod, ESP_LOG_INFO, "Sequence matched");
      auto spinnerFw = new SpinnerForwardAnimation(ledRingDemux);
      new Animator(
        *spinnerFw,
        200_ms
      );
    }

    vTaskDelay(pdMS_TO_TICKS(1_s));
  }
}

#endif  // RECEIVER
