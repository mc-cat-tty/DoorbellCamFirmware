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

  // xTaskNotifyFromISR(
  //   PWM_HANDLE,
  //   RX_EDGE_BIT,
  //   eSetBits,
  //   &higherPriorityTaskWoken
  // );

  ets_printf("Edge %d: %d\n", gpio_get_level(GPIO_NUM_23), xTaskGetTickCountFromISR());

  portEXIT_CRITICAL_ISR(&MUX);
  
  // portYIELD_FROM_ISR(higherPriorityTaskWoken);
}


void pwmCounterHandler(void *args) {
  auto tx = TxPwm(GPIO_NUM_18);
  tx.getTxTask().start("TxTask", 0, 8192);

  for (EVER) {
    tx.sendDutyAsync(0.1f);
    vTaskDelay(pdMS_TO_TICKS(4_s));
    tx.sendDutyAsync(0.2f);
    vTaskDelay(pdMS_TO_TICKS(4_s));
    tx.sendDutyAsync(0.3f);
    vTaskDelay(pdMS_TO_TICKS(4_s));
    tx.sendDutyAsync(0.4f);
    vTaskDelay(pdMS_TO_TICKS(4_s));
    tx.sendDutyAsync(0.5f);
    vTaskDelay(pdMS_TO_TICKS(4_s));
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

  xTaskCreatePinnedToCore(
    pwmCounterHandler,
    "pwmCounterHandler",
    10240,
    nullptr,
    0,
    &PWM_HANDLE,
    1
  );
 
  const auto rxPinConfig = (gpio_config_t) {
    .pin_bit_mask = 1ULL << GPIO_NUM_23,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };

  auto rxPin = Pin(
    GPIO_NUM_23,
    rxPinConfig
  );

  // gpio_install_isr_service(0);
  // gpio_isr_handler_add(
  //   GPIO_NUM_23,
  //   pwmCountIsr,
  //   nullptr
  // );


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

  uint64_t upCount = 0;
  uint64_t downCount = 0;

  typedef enum class State {
    ENTRY,
    WAITING,
    RECEIVING,
  } State;

  auto state = State::ENTRY;


  for (EVER) {
    // RX ISR
    // if (
    //   QUEUE != nullptr &&
    //   xQueueReceive(
    //     QUEUE,
    //     &upPercentage,
    //     pdMS_TO_TICKS(100_ms)
    //   )
    // ) {
    //   logger.log(mod, ESP_LOG_INFO, "Duty: %f\n", upPercentage);
    // }

    // if (!animator.isRunning()) {
    //   spinnerFw = SpinnerForwardAnimation(ledRingDemux);
    //   animator = Animator(spinnerFw, 200_ms);
    // }


    auto level = gpio_get_level(GPIO_NUM_23);
    int64_t rxStartTime;
    bool isTimeout;

    switch (state) {
      case State::ENTRY:
      upCount = 0;
      downCount = 0;

      if (level) {
        state = State::RECEIVING;
        rxStartTime = esp_timer_get_time();
      }
      else {
        state = State::WAITING;
      }

      break;

      case State::WAITING:
      upCount = 0;
      downCount = 0;
      
      if (level) {
        state = State::RECEIVING;
        upCount++;
        rxStartTime = esp_timer_get_time();
      }

      break;

      case State::RECEIVING:
      if (level) {
        upCount++;
      }
      else {
        downCount++;
      }

      isTimeout = esp_timer_get_time() - rxStartTime >= 2100e3;
      if (isTimeout) {
        state = State::WAITING;
      }

      break;

      default:
      state = State::ENTRY;
      break;
    }

    if (state == State::WAITING && upCount && downCount) {
      float duty = (float) upCount / (float) (upCount + downCount);
      logger.log(mod, ESP_LOG_INFO, "Duty: %f\n", roundf(duty*10));
    }

    vTaskDelay(pdMS_TO_TICKS(10_ms));
  }
}