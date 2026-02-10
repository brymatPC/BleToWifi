#ifndef BLE_PARSER_H
#define BLE_PARSER_H

#include <stdint.h>

enum class BleParserTypes: uint16_t {
  none = 0x0000,
  victron = 0x02E1,
  tempHumidity = 0x0010
};

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
    virtual void scanComplete() = 0;
};

#endif // BLE_PARSER_H