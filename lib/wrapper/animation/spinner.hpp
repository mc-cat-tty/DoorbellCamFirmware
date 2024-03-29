#pragma once
#include "animation.hpp"
#include <mux/demux.hpp>
#include <cmath>

using namespace hal::mux;

namespace wrapper::animation {
  enum class SpinnerStdDelay : time_t {
    DELAY_MS_5 = 5_ms,
    DELAY_MS_10 = 10_ms,
    DELAY_MS_25 = 25_ms,
    DELAY_MS_50 = 50_ms,
    DELAY_MS_100 = 100_ms,
  };

  class SpinnerAnimation : public IAnimation {
    protected:
    Demux demux;
    
    public:
    SpinnerAnimation(Demux demux)
      : demux{demux}
      {}
  };

  class SpinnerForwardAnimation : public SpinnerAnimation {
    private:
    size_t currentSelection = 0;
    
    public:
    using SpinnerAnimation::SpinnerAnimation;
    
    inline void animate() {
      const size_t selectionCount = pow(2ULL, demux.getSelCount());

      if (currentSelection < selectionCount) {
        demux.select(currentSelection++);
        return;
      }

      running = false;
    }
  };

  class SpinnerReverseAnimation : public SpinnerAnimation {
    private:
    int64_t currentSelection;
    
    public:
    SpinnerReverseAnimation(Demux demux)
      : SpinnerAnimation(demux),
      currentSelection{ (int64_t) pow(2ULL, demux.getSelCount()) - 1 }
      {}
    
    inline void animate() {
      if (currentSelection >= 0) {
        demux.select(currentSelection--);
        return;
      }

      running = false;
    }
  };

  class SpinnerLogMock : public SpinnerAnimation {
    private:
    size_t currentSelection = 0;

    public:
    using SpinnerAnimation::SpinnerAnimation;
    void animate();
  };

}