#pragma once
#include <initializer_list>
#include <math.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <task/task.hpp>
#include <time/udl.hpp>
#include <freertos/queue.h>

using namespace wrapper::task;

namespace hal::rf {
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

    [[nodiscard]] inline constexpr bool getDutyAsync(float &duty)
      const {
        return
          rxQueue != nullptr &&
          xQueueReceive(
            rxQueue,
            &duty,
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
    float currentDuty;
    uint8_t tolerance;

    public:
    RxSequence(
      RxPwm receiver,
      std::initializer_list<uint8_t> sequence,
      uint8_t tolerance) :
    receiver(receiver),
    sequence(sequence),
    tolerance(tolerance) { };

    [[nodiscard]] inline bool rcvdSequenceAsync() {
      long currentDutyInt = lroundf(currentDuty * 10.f);

      if (receiver.getDutyAsync(currentDuty)) {
        if (
          currentDutyInt < sequence[currentMatch] - tolerance ||
          currentDutyInt > sequence[currentMatch] + tolerance
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
}