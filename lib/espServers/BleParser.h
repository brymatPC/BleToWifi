#ifndef BLE_PARSER_H
#define BLE_PARSER_H

#include <stdint.h>

typedef struct {
  bool valid = false;
  char addr[20];
  uint8_t payloadLen = 0;
  uint8_t payload[50];
} bleDeviceData_t;

class BleParser {
public:
    virtual void setData(bleDeviceData_t &data) = 0;
    virtual void parse() = 0;
};

#endif // BLE_PARSER_H