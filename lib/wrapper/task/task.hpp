#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <functional>
#include <tuple>

namespace wrapper::task {
  template <typename CallingClass>
  class Task {
    private:
    std::function<CallingClass> implementation = NULL;
    TaskHandle_t handle = NULL;
    CallingClass *owner;

    [[noreturn]] void _task() {
      implementation(owner);
    }

    [[noreturn]] void task(void *params) {
      static_cast<wrapper::task::Task<CallingClass>*>(params)->_task();
    }

    public:
    Task(std::function<CallingClass> task_implementation, CallingClass method_owner)
      : implementation(task_implementation), owner(method_owner) {}

    [[nodiscard]] inline constexpr bool isRunning() const { return handle != NULL; }

    inline void start(const char *name, unsigned priority, unsigned stack_size) {
      if (!isRunning())
        xTaskCreate(task, name, stack_size, (void*)this, priority, &handle);
    }

    inline void stop() const {
      if (isRunning())
        vTaskDelete(handle);
    }
  };
}