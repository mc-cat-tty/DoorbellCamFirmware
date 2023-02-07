#pragma once
#include <initializer_list>
#include <math.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <task/task.hpp>
#include <time/udl.hpp>
#include <freertos/queue.h>

using namespace wrapper::task;
using namespace wrapper::log;


namespace hal::rf {
  typedef struct PwmData {
    float duty;
    uint64_t upCount;
    uint64_t downCount; 
  } PwmData;
 
  /**
   * @brief encoding state in duty cycle. See LEDC module.
   * 
   */
  class TxPwm {
    private:
    constexpr static const unsigned queue_dim = 10;
    constexpr static const unsigned tx_time = 2_s;
    constexpr static const unsigned tx_freq = 5_Hz;
    ledc_timer_bit_t duty_resolution;
    ledc_timer_config_t timer_conf;
    ledc_channel_config_t channel_conf;
    Task<hal::rf::TxPwm> tx_task;
    QueueHandle_t duty_queue;

    void setDutyAbs(unsigned long new_duty) const;
    [[noreturn]] void txTask();

    public:
    TxPwm(gpio_num_t pin_num);

    void setDutyPercentage(float duty_cycle_percentage); /** 0<=duty_cycle_percentage<=1 */
    
    inline void sendDutyAsync(float duty) {
      if (duty_queue != NULL) {
        xQueueSend(duty_queue, (void*) &duty, 0);
      }
    }

    [[nodiscard]] inline constexpr wrapper::task::Task<hal::rf::TxPwm> getTxTask()
      const { return tx_task; }
  };

  class RxPwm {
    private:
    constexpr static const unsigned queueDim = 10;
    constexpr static const unsigned timeoutMargin = 100_ms;
    constexpr static const unsigned timeoutMs = 2_s + timeoutMargin;
    gpio_num_t pinNum;
    QueueHandle_t rxQueue;
    Task<hal::rf::RxPwm> rxTaskHandle;

    [[noreturn]] void rxTask();

    public:
    RxPwm(gpio_num_t pinNum);

    [[nodiscard]] inline constexpr bool getPwmDataAsync(PwmData &data)
      const {
        return
          rxQueue != nullptr &&
          xQueueReceive(
            rxQueue,
            &data,
            0
          );
      }

    [[nodiscard]] inline constexpr Task<hal::rf::RxPwm> getRxTask() const {
      return rxTaskHandle;
    }
  };

  class TxSequence {
    private:
    TxPwm transmitter;
    std::vector<uint8_t> sequence;
    time_t txDelayMs;

    public:
    TxSequence(
      TxPwm transmitter,
      std::initializer_list<uint8_t> sequence,
      time_t txDelayMs) :
      transmitter(transmitter),
      sequence(sequence),
      txDelayMs(txDelayMs) { };
    
    inline void sendSequence() {
      for (const auto &num : sequence) {
        transmitter.sendDutyAsync(num / 10.f);
        vTaskDelay(pdMS_TO_TICKS(txDelayMs));
      }
    }
  };

  class RxSequence {
    private:
    RxPwm receiver;
    std::vector<uint8_t> sequence;
    uint8_t currentMatch = 0;
    PwmData currentData;
    uint8_t tolerance;
    uint64_t upCountThreshold;

    public:
    RxSequence(
      RxPwm receiver,
      std::initializer_list<uint8_t> sequence,
      uint8_t tolerance,
      uint64_t upCountThreshold) :
    receiver(receiver),
    sequence(sequence),
    tolerance(tolerance),
    upCountThreshold(upCountThreshold) { };

    [[nodiscard]] inline bool rcvdSequenceAsync() {
      long prevDutyInt = lroundf(currentData.duty * 10.f);

      if (receiver.getPwmDataAsync(currentData) && currentData.upCount > upCountThreshold) {
        if (
          prevDutyInt < sequence[currentMatch] - tolerance ||
          prevDutyInt > sequence[currentMatch] + tolerance
        ) {
          currentMatch = 0;
          return false;
        }

        currentMatch++;
        currentMatch %= sequence.size();

        return currentMatch == 0;  // Return if matched an entire sequence
      }

      return false;
    }
  };

  class RxDeltas {
    private:
    RxPwm receiver;
    std::vector<float> dutyDeltaSequence;
    uint8_t currentMatch = 0;
    PwmData currentData;
    float tolerance;
    uint64_t upCountThreshold;

    public:
    RxDeltas(
      RxPwm receiver,
      std::initializer_list<float> dutyDeltaSequence,
      float tolerance,
      uint64_t upCountThreshold) :
    receiver(receiver),
    dutyDeltaSequence(dutyDeltaSequence),
    tolerance(tolerance),
    upCountThreshold(upCountThreshold) { };

    [[nodiscard]] inline bool rcvdDeltasAsync() {
      static const auto logger = Logger::getInstance();
      static const auto mod = Module::RF;

      float prevDuty = currentData.duty;

      if (receiver.getPwmDataAsync(currentData) && currentData.upCount > upCountThreshold) {

        logger.log(mod, ESP_LOG_DEBUG, "Delta: %f\n", currentData.duty - prevDuty);

        if (
          currentData.duty - prevDuty < dutyDeltaSequence[currentMatch] - tolerance ||
          currentData.duty - prevDuty > dutyDeltaSequence[currentMatch] + tolerance
        ) {
          currentMatch = 0;
          return false;
        }

        currentMatch++;
        currentMatch %= dutyDeltaSequence.size();

        return currentMatch == 0;  // Return if matched an entire sequence
      }

      return false;
    }
  };

}