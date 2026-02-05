#ifndef LED_STRIP_DRIVER_H
#define LED_STRIP_DRIVER_H

#include <stdint.h>
#include <driver/rmt_tx.h>

// Code copied from esp-idf examples: https://github.com/espressif/esp-idf/blob/release/v6.0/examples/peripherals/rmt/led_strip_simple_encoder/main/led_strip_example_main.c

class DebugLog;

class LedStripDriver {
private:
    rmt_channel_handle_t m_ledChan;
    rmt_encoder_handle_t m_encoder;

    DebugLog* m_log;
public:
    LedStripDriver();
    virtual ~LedStripDriver();
    void setup(DebugLog *log);
    void setLed(uint32_t pixelVal);
};

#endif //  LED_STRIP_DRIVER_H