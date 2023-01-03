#include "rf.hpp"
#include <log/log.hpp>
#include <common.hpp>
#include <time/udl.hpp>
#include <stdexcept>
#include <math.h>

#define EVER ;;

using namespace hal::rf;

constexpr static const wrapper::log::Module mod = wrapper::log::Module::RF;

TxPwm::TxPwm(gpio_num_t pin_num) : tx_task(&hal::rf::TxPwm::txTask, this) {
  duty_queue = xQueueCreate(queue_dim, sizeof(float));

  duty_resolution = LEDC_TIMER_10_BIT;

  ledc_timer_config_t timer_conf_local = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .duty_resolution = duty_resolution,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = tx_freq,
    .clk_cfg = LEDC_AUTO_CLK,
  };
  timer_conf = timer_conf_local;
  ledc_timer_config(&timer_conf);

  ledc_channel_config_t channel_conf_local = {
    .gpio_num = pin_num,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,
    .hpoint = 0,
    .flags = { .output_invert = 0 },
  };
  channel_conf = channel_conf_local;
  ledc_channel_config(&channel_conf);
  
  static const wrapper::log::Logger logger = wrapper::log::Logger::getInstance();
  logger.log(mod, ESP_LOG_INFO, "Configured LEDC on gpio %d", pin_num);
}

void TxPwm::setDutyAbs(unsigned long new_duty) const {
  ledc_set_duty(channel_conf.speed_mode, channel_conf.channel, new_duty);
  ledc_update_duty(channel_conf.speed_mode, channel_conf.channel);
}

void TxPwm::setDutyPercentage(float duty_cycle_percentage) {
  duty_cycle_percentage = duty_cycle_percentage > 1.f ? 1.f : duty_cycle_percentage;
  duty_cycle_percentage = duty_cycle_percentage < 0.f ? 0.f : duty_cycle_percentage;
  static const unsigned long max_duty = powl(2, duty_resolution) - 1;
  setDutyAbs(duty_cycle_percentage * max_duty);
}

[[noreturn]] void TxPwm::txTask() {
  static const wrapper::log::Logger logger = wrapper::log::Logger::getInstance();
  static float rcv_duty;

  for (EVER) {
    const bool duty_received =
      duty_queue != NULL &&
      xQueueReceive(duty_queue, &rcv_duty, 5);
    
    if (duty_received) {
      logger.log(mod, ESP_LOG_DEBUG, "Sending duty=%f", rcv_duty);
      setDutyPercentage(rcv_duty);
      ledc_timer_resume(channel_conf.speed_mode, channel_conf.timer_sel);
      vTaskDelay(pdMS_TO_TICKS(tx_time));
      ledc_stop(channel_conf.speed_mode, channel_conf.channel, 0);
      ledc_timer_pause(channel_conf.speed_mode, channel_conf.timer_sel);
    }

    vTaskDelay(pdMS_TO_TICKS(1000.f / tx_freq));  // Ms in a period
  }
}