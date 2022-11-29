#pragma once
#include "pin/pin.hpp"
#include "pin/pin_state.hpp"
#include <cstdint>
#include <initializer_list>
#include <vector>
#include <driver/gpio.h>

using namespace hal::pin;

namespace hal::mux {
  class Demux {
    protected:
    Pin inputPin;
    std::vector<Pin> selectorPins;

    public:
    Demux(Pin input, std::initializer_list<Pin> selectors)
      : inputPin{input},
      selectorPins{selectors}
      {}
    
    Demux(std::initializer_list<Pin> selectors)
      : inputPin(GPIO_NUM_MAX),
      selectorPins{selectors}
      {}
    
    inline void setInput(State::In demuxInputState) {
      inputPin.setState(demuxInputState);
    }

    inline void select(uint64_t selectionBits) {

      for (int i = 0; i < selectorPins.size(); i++) {
        
        selectorPins[i].setState(
          State::fromIntToIn(
            (selectionBits >> i) & 0x01
          )
        );

      }

    }

    [[nodiscard]] inline constexpr size_t getSelCount()
      const { return selectorPins.size(); }

  };
}