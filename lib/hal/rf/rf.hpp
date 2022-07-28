#pragma once
#include <driver/gpio.h>
#include <driver/ledc.h>

namespace hal::rf {
  /**
   * @brief encoding state in duty cycle. See LEDC module.
   * 
   */
  class TxPwm {
    private:
    ledc_timer_bit_t duty_resolution;
    ledc_timer_config_t timer_conf;
    ledc_channel_config_t channel_conf;

    void setDuty(unsigned long new_duty) const;

    public:
    TxPwm(gpio_num_t pin_num);
    void setDutyPercentage(float duty_cycle_percentage); /** 0<=duty_cycle_percentage<=1 */
  };

  class RxPwm {

  };
}