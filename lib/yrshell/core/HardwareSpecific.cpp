#include "HardwareSpecific.h"
#include <esp32-hal.h>

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


