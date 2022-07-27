#pragma once
#include <string>
#include <time/udl.hpp>
#include <pin/pin.hpp>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace hal::led {
  class Led : public hal::pin::Pin {
    private:
    int blink_delay = 0_s;
    TaskHandle_t blink_handle = NULL;
    std::string blink_task_name = std::string("blinkLedPin") + std::to_string(num);

    [[noreturn]] void __blinkTask();
    [[noreturn]] static void blinkTask(void *params);

    public:
    using hal::pin::Pin::Pin;

    [[nodiscard]] constexpr int getBlinkDelay() const { return blink_delay; }
    void setBlinkDelay(int delay) { blink_delay = delay; }

    inline void startBlink(unsigned priority) {
      if (!isBlinking())
        xTaskCreate(blinkTask, blink_task_name.c_str(), 4096, (void*)this, priority, &blink_handle);
    }
    [[nodiscard]] inline constexpr bool isBlinking() const { return blink_handle != NULL; };
    inline void stopBlink() const {
      if (isBlinking())
        vTaskDelete(blink_handle);
    }
  };
}