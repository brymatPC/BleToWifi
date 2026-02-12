#ifndef LED_DRIVER_H_
#define LED_DRIVER_H_

#include <stdint.h>

class LedDriver {
public:
  virtual void setLedOnOffMs(uint32_t on, uint32_t off) = 0;
  virtual void blink(uint32_t timeMs) = 0;
  virtual void on() = 0;
  virtual void off() = 0;
  virtual void push() = 0;
  virtual void pop() = 0;
};

#endif // LED_DRIVER_H_
