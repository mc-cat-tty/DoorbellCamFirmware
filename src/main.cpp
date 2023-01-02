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

auto QUEUE = xQueueCreate(10, sizeof(float));
TaskHandle_t RX_HANDLE;
static const auto RX_EDGE_BIT = 0X01;

static void IRAM_ATTR pwmCountIsr(void *args) {
  BaseType_t higherPriorityTaskWoken = pdFALSE;

  xTaskNotifyFromISR(
    RX_HANDLE,
    RX_EDGE_BIT,
    eSetBits,
    &higherPriorityTaskWoken
  );
  
  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}


void txTask(void *args) {
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

void rxTask(void *args) {
  static uint64_t upCount = 0;
  static uint64_t downCount = 0;

  typedef enum class State {
    WAITING,
    RECEIVING,
  } State;

  static auto state = State::WAITING;

  auto level = gpio_get_level(GPIO_NUM_23);
  static int64_t rxStartTime = esp_timer_get_time();
  bool isTimeout;
  uint32_t notifiedVal;
  bool canStartRx;

  for (EVER) {
    switch (state) {
      case State::WAITING:
      upCount = 0;
      downCount = 0;
      
      // canStartRx =
      //   xTaskNotifyWait(pdFALSE, ULONG_MAX, &notifiedVal, portMAX_DELAY)
      //   ==
      //   pdTRUE;
      
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
      state = State::WAITING;
      break;
    }

    if (state == State::WAITING && upCount && downCount) {
      float duty = (float) upCount / (float) (upCount + downCount);
      // duty = roundf(duty * 10);
      xQueueSend(
        QUEUE,
        &duty,
        0
      );
    }

    vTaskDelay(pdMS_TO_TICKS(10_ms));
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
    txTask,
    "txTask",
    10240,
    nullptr,
    0,
    nullptr,
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

  xTaskCreatePinnedToCore(
    rxTask,
    "rxTask",
    1024,
    nullptr,
    configMAX_PRIORITIES - 1,
    &RX_HANDLE,
    0
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

  float duty;
  for (EVER) {
    if (
      QUEUE != nullptr &&
      xQueueReceive(
        QUEUE,
        &duty,
        portMAX_DELAY
      )
    ) {
      logger.log(mod, ESP_LOG_INFO, "Duty: %f\n", duty);
    }

    // if (!animator.isRunning()) {
    //   spinnerFw = SpinnerForwardAnimation(ledRingDemux);
    //   animator = Animator(spinnerFw, 200_ms);
    // }
  }
}