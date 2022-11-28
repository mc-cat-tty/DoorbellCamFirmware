#pragma once
#include <stdexcept>

namespace hal::pin {
  class State {
    public:
    enum class Out {
      LOW,
      HIGH,
    };

    enum class In {
      LOW,
      HIGH,
      TOGGLE,
    };

    [[nodiscard]] inline static constexpr Out fromIntToOut(int level) { return static_cast<Out>(level); }
    [[nodiscard]] inline static constexpr int fromOutToInt(Out state) { return static_cast<int>(state); }
    [[nodiscard]] inline static constexpr In fromIntToIn(int level) { return static_cast<In>(level); }
    
    [[nodiscard]] inline static int fromIntoInt(In state) {
      if (state == In::TOGGLE) {
        throw std::invalid_argument("Invalid input state");
      }

      return static_cast<int>(state);
    }

    [[nodiscard]] inline static Out fromInToOut(In inState) {
      if (inState == In::TOGGLE) {
        throw std::invalid_argument("Invalid input state");
      }

      return static_cast<Out>(inState);
    }
  };
}
