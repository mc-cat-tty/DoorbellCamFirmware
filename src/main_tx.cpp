#ifdef TRANSMITTER

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
using namespace hal::rf;

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

  auto tx = TxPwm(GPIO_NUM_18);
  tx.getTxTask().start("TxTask", 0, 8192);

  auto sequenceTx = TxSequence(
    tx,
    {4, 5, 4},
    1_s
  );
  
  sequenceTx.sendSequence();
}

#endif  // TRANSMITTER