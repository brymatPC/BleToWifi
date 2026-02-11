#include "HardwareSpecific.h"
#include "BufferedSerial.h"
#include <esp32-hal.h>

void HW_setSerialBaud( uint32_t port, uint32_t baud) {
    switch( port) {
#ifdef ENABLE_SERIAL
	case 0:
		BSerial.setBaud( baud);
	break;
#endif
#ifdef ENABLE_SERIAL1
	case 1:
		BSerial1.setBaud( baud);
	break;
#endif
#ifdef ENABLE_SERIAL2
	case 2:
		BSerial2.setBaud( baud);
	break;
#endif
#ifdef ENABLE_SERIAL3
	case 3:
		BSerial3.setBaud( baud);
	break;
#endif
	default:
	break;
	}
}


uint32_t HW_getSysticks() {
    return (uint32_t)(esp_timer_get_time()); 
}
uint32_t HW_getMicros() {
    return (uint32_t)(esp_timer_get_time()); 
}
uint32_t HW_getMillis() {
    return (uint32_t)(esp_timer_get_time() / 1000ULL); 
} 
uint32_t HW_getSysTicksPerSecond( ) {
	return 1000000;
}


