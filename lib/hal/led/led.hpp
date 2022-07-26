#pragma once
#include <string>
#include <time/udl.hpp>
#include <pin/pin.hpp>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace hal::led {
  class Led : hal::pin::Pin {
    private:
    int blink_delay = 0_s;
    TaskHandle_t blink_handle = NULL;
    std::string object_name = std::string("pin") + std::to_string(num);

    [[noreturn]] void __blinkTask();
    [[noreturn]] static void blinkTask(void *params);

    public:
    using hal::pin::Pin::Pin;

    [[nodiscard]] constexpr int getBlinkDelay() const { return blink_delay; }
    void setBlinkDelay(int delay) { blink_delay = delay; }

    void startBlink(unsigned priority)
      { if (!isBlinking()) xTaskCreate(blinkTask, object_name.c_str(), 512, (void*)this, priority, &blink_handle); }
    [[nodiscard]] constexpr bool isBlinking() const { return blink_handle != NULL; };
    void stopBlink() const { if (isBlinking()) vTaskDelete(blink_handle); }
  };
}