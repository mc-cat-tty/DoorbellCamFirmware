#include "pin.hpp"
#include <log/log.hpp>
#include <common.hpp>
#include <stdexcept>

using namespace hal::pin;

constexpr static const wrapper::log::Module mod = wrapper::log::Module::PIN;

[[nodiscard]] bool Pin::trySetState(State::In target_state) const {
  static const wrapper::log::Logger logger = wrapper::log::Logger::getInstance();

  const State::Out current_state = getState();
  const bool canExit = 
    target_state != State::In::TOGGLE &&
    current_state == State::fromInToOut(target_state);
  
  logger.log(
    mod,
    ESP_LOG_DEBUG,
    "%d - target_state: %d",
    (int) num, (int) target_state
  );

  if (canExit)
    return true;
  
  gpio_set_level(num, !State::fromOutToInt(current_state));
  const State::Out future_state = getState();
  
  logger.log(
    mod,
    ESP_LOG_DEBUG,
    "%d - current_state:%d\tfuture_state:%d",
    (int) num, (int) current_state, (int) future_state
  );
  
  if (future_state != current_state)
    return true;


  return false;
}

void Pin::setState(State::In state) {
  int error_counter = 0;
  constexpr static const int max_err_count = 5;
  while (error_counter < max_err_count && !Pin::trySetState(state)) {
    error_counter++;
  }

  if (error_counter >= max_err_count)
    throw std::runtime_error("Max retry number");
}

bool Pin::uncheckedSetState(State::In state) {
  if (gpio_set_level(num, State::fromInToInt(state)) != ESP_OK) {
    return false;
  }

  return true;
}