#include "rf.hpp"
#include <log/log.hpp>
#include <pin/pin.hpp>
#include <math.h>

#define EVER ;;
 
using namespace hal::rf;
using namespace hal::pin;
using namespace wrapper::log;

constexpr static const auto mod = Module::RF;

RxPwm::RxPwm(gpio_num_t pinNum)
  : pinNum(pinNum),
  rxQueue(xQueueCreate(queueDim, sizeof(PwmData))),
  rxTaskHandle(&RxPwm::rxTask, this)
  {
  const auto pinConfig = (gpio_config_t) {
    .pin_bit_mask = 1ULL << pinNum,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };

  Pin(
    pinNum,
    pinConfig
  );
}

[[noreturn]] void RxPwm::rxTask() {
  static const auto logger = Logger::getInstance();
  constexpr const unsigned samplesNumInPeriod = 20;
  uint64_t upCount = 0;
  uint64_t downCount = 0;
  int64_t rxStartTick;
  bool isTimeout;
  uint32_t notifiedVal;
  bool canStartRx;

  typedef enum class State {
    WAITING,
    RECEIVING,
  } State;
  
  auto state = State::WAITING;

  for (EVER) {
    auto level = gpio_get_level(pinNum);
    
    switch (state) {
      case State::WAITING:
      upCount = 0;
      downCount = 0;

      if (level) {
        state = State::RECEIVING;
        upCount++;
        rxStartTick = xTaskGetTickCount();
      }

      break;

      case State::RECEIVING:
      if (level) {
        upCount++;
      }
      else {
        downCount++;
      }

      isTimeout = pdTICKS_TO_MS(xTaskGetTickCount() - rxStartTick) >= timeoutMs;
      if (isTimeout) {
        state = State::WAITING;
        downCount -= samplesNumInPeriod;  // samples number in a period
      }
      break;

      default:
      state = State::WAITING;
      break;
    }

    // constexpr const unsigned filterThreshold = 180;
    if (
      state == State::WAITING &&
      upCount && downCount
    ) {
      const float duty = (float) upCount / (float) (upCount + downCount);

      const auto data = (PwmData) {
        .duty = duty,
        .upCount = upCount,
        .downCount = downCount
      };

      logger.log(mod, ESP_LOG_INFO, "Rx duty: %f\n", duty);
      logger.log(mod, ESP_LOG_DEBUG, "Up count: %d\n", upCount);
      logger.log(mod, ESP_LOG_DEBUG, "Down count: %d\n", downCount);

      if (rxQueue != nullptr) {
        xQueueSend(
          rxQueue,
          &data,
          0
        );
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10_ms));
  }
}
