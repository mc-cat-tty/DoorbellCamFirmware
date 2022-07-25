#pragma once
#include "pin_state.hpp"
#include "driver/gpio.h"

namespace hal::pin {
  class Pin {
    private:
    gpio_config_t conf;
    gpio_num_t num;
    State::Out current_state = State::Out::LOW;
    esp_err_t err_state = ESP_OK;

    [[nodiscard]] bool trySetState(State::In target_state);

    public:
    Pin(gpio_num_t pin_num, const gpio_config_t* pin_config)
      : num(pin_num), conf(*pin_config) { err_state = gpio_config(pin_config); }

    [[noreturn]] void setState(State::In state);

    [[nodiscard]] constexpr bool isOk() const { return err_state == ESP_OK; }
    [[nodiscard]] State::Out getState() { return current_state = State::fromInt(gpio_get_level(num)); }

  };
}