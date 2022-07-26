#include "pin.hpp"
#include <stdexcept>

using namespace hal::pin;

[[nodiscard]] bool Pin::trySetState(State::In target_state) const {
  const State::Out current_state = getState();
  const bool canExit = 
    target_state != State::In::TOGGLE &&
    current_state == State::fromIn(target_state);

  if (canExit)
    return true;
  
  gpio_set_level(num, !State::toInt(current_state));
  const State::Out future_state = getState();
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