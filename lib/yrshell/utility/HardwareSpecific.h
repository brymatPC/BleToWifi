#ifndef HardwareSpecific_h
#define HardwareSpecific_h

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define SERIAL_FLOW_CONTROL_NONE 0
#define SERIAL_FLOW_CONTROL_XON_XOFF 1
#define SERIAL_FLOW_CONTROL_RTS_CTS 2

void HW_setSerialBaud( uint32_t port, uint32_t baud);

uint32_t HW_getSysticks( void);
uint32_t HW_getMicros( void);
uint32_t HW_getMillis( void);

uint32_t HW_getSysTicksPerSecond( void);

#endif
