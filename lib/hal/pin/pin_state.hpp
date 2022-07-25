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

    [[nodiscard]] static constexpr Out fromInt(int level) { return static_cast<Out>(level); }
    [[nodiscard]] static constexpr int toInt(Out state) { return static_cast<int>(state); }
    static Out fromIn(In inState) {
      if (inState == In::TOGGLE)
        throw std::invalid_argument("Invalid input state");
      return static_cast<Out>(inState);
    }
  };
}
