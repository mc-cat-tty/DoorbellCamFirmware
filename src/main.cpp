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
#include <driver/mcpwm.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define EVER ;;

using namespace hal::pin;
using namespace hal::led;
using namespace hal::mux;
using namespace hal::rf;
using namespace wrapper::animation;

constexpr static const wrapper::log::Module mod = wrapper::log::Module::MAIN;

uint32_t PWM_VALUE;
time_t RISING_TIME_1 = 0ULL;
time_t FALLING_TIME_2 = 0ULL;
time_t RISING_TIME_3 = 0ULL;
auto QUEUE = xQueueCreate(1000, sizeof(float));
TaskHandle_t PWM_HANDLE;
static const auto RX_EDGE_BIT = 0X01;
portMUX_TYPE MUX = portMUX_INITIALIZER_UNLOCKED;
uint32_t DEBOUNCE_VECTOR = 0;
typedef enum {
  RISING1,
  FALLING2,
  RISING3,
  FALLING4,
} StateFsm;
StateFsm fsmState = RISING1;

static void IRAM_ATTR pwmCountIsr(void *args) {
  // PWM_VALUE = mcpwm_capture_signal_get_value(MCPWM_UNIT_0, MCPWM_SELECT_CAP0);

  BaseType_t higherPriorityTaskWoken = pdFALSE;

  portENTER_CRITICAL_ISR(&MUX);

  xTaskNotifyFromISR(
    PWM_HANDLE,
    RX_EDGE_BIT,
    eSetBits,
    &higherPriorityTaskWoken
  );

  portEXIT_CRITICAL_ISR(&MUX);
  
  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void pwmCounterHandler(void *args) {
  uint32_t notification;
  uint8_t gpioVal;

  for (EVER) {
    const auto passed = xTaskNotifyWait(
      false,
      RX_EDGE_BIT,
      &notification,
      pdMS_TO_TICKS(10_ms)
    );

    if (passed && (notification & RX_EDGE_BIT) ) {
      gpioVal = gpio_get_level(GPIO_NUM_23);
      
      switch(fsmState) {
        case RISING1:
        RISING_TIME_1 = xTaskGetTickCount();
        if (!gpioVal) {
          fsmState = FALLING2;
        }
        break;

        case FALLING2:
        FALLING_TIME_2 = xTaskGetTickCount();
        if (gpioVal) {
          fsmState = RISING3;
        }
        break;

        case RISING3:
        RISING_TIME_3 = xTaskGetTickCount();
        if (!gpioVal) { 
          fsmState = FALLING4;
        }
        break;

        case FALLING4:
        if (RISING_TIME_3 - RISING_TIME_1 != 0) {
          float duty = (float) (FALLING_TIME_2 - RISING_TIME_1) / (float) (RISING_TIME_3 - RISING_TIME_1);
          xQueueSend(QUEUE, (void *) &duty, 0);
        }
        if (gpioVal) {
          fsmState = RISING1;
        }
        break;
      }

    }
  }
}

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

  // gpio_config_t builtin_led_config = {
  //   .pin_bit_mask = 1ULL << GPIO_NUM_2,
  //   .mode = GPIO_MODE_INPUT_OUTPUT,
  //   .pull_up_en = GPIO_PULLUP_DISABLE,
  //   .pull_down_en = GPIO_PULLDOWN_DISABLE,
  //   .intr_type = GPIO_INTR_DISABLE,
  // };
  // hal::led::Led builtin_led(GPIO_NUM_2, builtin_led_config);
  // if (!builtin_led.isOk())
  //   logger.log(mod, ESP_LOG_ERROR, "Error while building builtin_led");
  
  // static const int blink_delay = 0.5_s;
  // builtin_led.setBlinkDelay(blink_delay);
  // logger.log(mod, ESP_LOG_INFO, "blink_delay = %d ms", blink_delay);
  // builtin_led.getBlinkTask().start("BuiltinLedTask", 0, 4096);

  auto tx = TxPwm(GPIO_NUM_18);
  tx.getTxTask().start("TxTask", 0, 8192);

  const auto max_duty = 10;
  const auto duty_increment = 2;
  static auto current_duty = 0;

  xTaskCreate(
    pwmCounterHandler,
    "pwmCounterHandler",
    1024,
    nullptr,
    configMAX_PRIORITIES - 1,
    &PWM_HANDLE
  );
 
  const auto rxPinConfig = (gpio_config_t) {
    .pin_bit_mask = 1ULL << GPIO_NUM_23,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_ANYEDGE,
  };

  auto rxPin = Pin(
    GPIO_NUM_23,
    rxPinConfig
  );

  gpio_install_isr_service(0);
  gpio_isr_handler_add(
    GPIO_NUM_23,
    pwmCountIsr,
    nullptr
  );

  // const auto ledsConfig = (gpio_config_t) {
  //   .pin_bit_mask =
  //     1ULL << 25 |
  //     1ULL << 26 |
  //     1ULL << 32 |
  //     1ULL << 33,
  //   .mode = GPIO_MODE_OUTPUT,
  //   .pull_up_en = GPIO_PULLUP_DISABLE,
  //   .pull_down_en = GPIO_PULLDOWN_DISABLE,
  //   .intr_type = GPIO_INTR_DISABLE,
  // };

  // auto ledRingDemux = Demux{
  //   Led(GPIO_NUM_33, ledsConfig),
  //   Led(GPIO_NUM_25, ledsConfig),
  //   Led(GPIO_NUM_32, ledsConfig),
  //   Led(GPIO_NUM_26, ledsConfig),
  // };

  // auto spinnerFw = SpinnerForwardAnimation(ledRingDemux);
  // auto animator = Animator(spinnerFw, 200_ms);
  // logger.log(mod, ESP_LOG_DEBUG, "First animation run");

  float upPercentage;

  for (EVER) {
    tx.sendDutyAsync((float)current_duty/10);
    current_duty += duty_increment;
    current_duty %= max_duty;

    if (
      QUEUE != nullptr &&
      xQueueReceive(
        QUEUE,
        &upPercentage,
        pdMS_TO_TICKS(100_ms)
      )
    ) {
      logger.log(mod, ESP_LOG_INFO, "Duty: %f\n", upPercentage);
    }

    // if (!animator.isRunning()) {
    //   spinnerFw = SpinnerForwardAnimation(ledRingDemux);
    //   animator = Animator(spinnerFw, 200_ms);
    // }
  }
}