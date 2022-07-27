#include "led.hpp"
#include <common.hpp>
#include <log/log.hpp>
#include <exception>

#define EVER ;;

using namespace hal::led;

constexpr static const wrapper::log::Module mod = wrapper::log::Module::LED;

[[noreturn]] void Led::blinkTask() {
  static const wrapper::log::Logger logger = wrapper::log::Logger::getInstance();

  for (EVER) {
    try {
      setState(hal::pin::State::In::TOGGLE);
      logger.log(mod, ESP_LOG_DEBUG, "Toggled led %d", num);
      vTaskDelay(pdMS_TO_TICKS(blink_delay));
    }
    catch (const std::exception &e) {
      logger.log(mod, ESP_LOG_ERROR, e.what());
    }
  }
}
