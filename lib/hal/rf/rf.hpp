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
    ledc_timer_config_t timer_conf;
    ledc_channel_config_t channel_conf;

    public:
    TxPwm(gpio_num_t pin_num);

    void setDuty(unsigned long new_duty) const;
  };

  class RxPwm {

  };
}