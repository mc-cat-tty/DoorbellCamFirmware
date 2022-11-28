#pragma once
#include "pin_state.hpp"
#include <driver/gpio.h>

namespace hal::pin {
  class Pin {
    private:
    gpio_config_t conf;

    [[nodiscard]] bool trySetState(State::In target_state) const;

    protected:
    gpio_num_t num;
    esp_err_t err_state = ESP_OK;

    public:
    Pin(gpio_num_t pin_num, const gpio_config_t& pin_config)
      : conf(pin_config), num(pin_num) { err_state = gpio_config(&pin_config); }

    void setState(State::In state);

    [[nodiscard]] inline constexpr bool isOk() const { return err_state == ESP_OK; }
    [[nodiscard]] inline State::Out getState() const { return State::fromIntToOut(gpio_get_level(num)); }

  };
}