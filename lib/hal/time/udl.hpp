#pragma once
#include <freertos/FreeRTOS.h>

constexpr unsigned long long operator"" _s(unsigned long long t) {
  return t*1000;
}

constexpr unsigned long long operator"" _s(long double t) {
  return t*1000;
}

constexpr unsigned long long operator"" _ms(unsigned long long t) {
  return t;
}

constexpr unsigned long long operator"" _ticks(unsigned long long t) {
  return pdTICKS_TO_MS(t);
}