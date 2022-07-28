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

constexpr unsigned long long operator"" _Hz(unsigned long long f) {
  return f;
}

constexpr unsigned long long operator"" _kHz(unsigned long long f) {
  return f*10e3;
}

constexpr unsigned long long operator"" _MHz(unsigned long long f) {
  return f*10e6;
}