#include "rf.hpp"
#include <driver/gpio.h>
#include <driver/ledc.h>

namespace hal::rf {
  /**
   * @brief encoding state in duty cycle. See LEDC module.
   * 
   */
  class Tx {
    private:
    ledc_timer_config_t timer_conf;
    ledc_channel_config_t channel_conf;

    public:
    Tx(gpio_num_t pin_num) {
      timer_conf = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = REF_CLK_FREQ/4,
        .clk_cfg = LEDC_AUTO_CLK,
      };
      ledc_timer_config(&timer_conf);

      channel_conf = {
        .gpio_num = pin_num,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
      };
      ledc_channel_config(&channel_conf);
    }

    inline void setDuty(unsigned long new_duty) const {
      ledc_set_duty(channel_conf.speed_mode, channel_conf.channel, new_duty);
      ledc_update_duty(channel_conf.speed_mode, channel_conf.channel);
    }
  };
}