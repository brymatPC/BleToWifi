#ifndef LED_STRIP_DRIVER_H
#define LED_STRIP_DRIVER_H

#include <stdint.h>
#include <driver/rmt_tx.h>
#include <core/Sliceable.h>
#include <utility/LedDriver.h>
#include <core/IntervalTimer.h>

// Code copied from esp-idf examples: https://github.com/espressif/esp-idf/blob/release/v6.0/examples/peripherals/rmt/led_strip_simple_encoder/main/led_strip_example_main.c

class DebugLog;

class LedStripDriver : public Sliceable, public LedDriver {
private:
    static const uint32_t s_DEFAULT_COLOUR;

    rmt_channel_handle_t m_ledChan;
    rmt_encoder_handle_t m_encoder;

    DebugLog* m_log;

    uint32_t m_colour;
    int8_t m_tos;
    uint32_t m_stack[16];
    uint32_t m_ledOnMs;
    uint32_t m_ledOffMs;
    bool m_ledState;
    IntervalTimer m_ledTimer;

    void transmit(uint32_t pixelVal);

public:
    LedStripDriver();
    virtual ~LedStripDriver();
    virtual const char* sliceName( ) { return "LedStripDriver"; }
    virtual void slice();
    void setup(DebugLog *log);
    void setLed(uint32_t pixelVal) {m_colour = pixelVal; }

    void setLedOnOffMs( uint32_t on, uint32_t off);
    void blink( uint32_t timeMs) { setLedOnOffMs( timeMs, timeMs); }
    void on( void) { setLedOnOffMs( 0xFFFFFFF, 1); }
    void off( void) { setLedOnOffMs( 1, 0xFFFFFFF); }
    void push();
    void pop();
};

#endif //  LED_STRIP_DRIVER_H