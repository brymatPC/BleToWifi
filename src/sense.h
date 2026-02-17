#ifndef SENSE_H_
#define SENSE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void sensirion_i2c_hal_init_ext(void);
int8_t sensirion_i2c_hal_read_ext(uint8_t address, uint8_t* data, uint16_t count);
int8_t sensirion_i2c_hal_write_ext(uint8_t address, const uint8_t* data, uint16_t count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif