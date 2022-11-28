#pragma once
#include <ctime>
#include <cstdint>
#include <time/udl.hpp>
#include <task/task.hpp>

using namespace wrapper::task;

namespace app::animation {
  class IAnimation {
    protected:
      bool running = true;
      bool stopping = false;
    
    public:
      virtual void animate() = 0;
      
      virtual inline bool isRunning() const {
        return running;
      }
      
      virtual inline bool tryStop() {
        stopping = true;
        return isRunning();
      }

  };

  class Animator {
    protected:
      IAnimation *animationService;
      time_t refreshDelay;
      time_t stopTimeout;
      Task<app::animation::Animator> animationTask;

      inline void loop() {
        while (animationService->isRunning()) {
          animationService->animate();
          vTaskDelay(pdMS_TO_TICKS(refreshDelay));
        }

        vTaskDelete(nullptr);
      }
    
    public:
      Animator(IAnimation *animationService, time_t refreshDelay, time_t stopTimeout = 500_ms)
        : animationService{animationService},
        refreshDelay{refreshDelay},
        stopTimeout{stopTimeout},
        animationTask(&app::animation::Animator::loop, this)
        {
          animationTask.start("AnimationTask", 0, 4096);
        }
      
      ~Animator() {
        const time_t toWait = 10_ms;
        time_t passed = 0;

        while (!animationService->tryStop() && passed < stopTimeout) {
          vTaskDelay(pdMS_TO_TICKS(toWait));
          passed += toWait;
        }

        animationTask.stop();
      }
  
  };
}