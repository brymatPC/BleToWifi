#ifndef TEMP_HUMIDITY_SENSOR_H
#define TEMP_HUMIDITY_SENSOR_H

#include <stdint.h>
#include <BleParser.h>

class DebugLog;

class TempHumiditySensor : public BleParser {
private:
    DebugLog* m_log;
    bleDeviceData_t m_data;

    uint8_t processBatteryVoltage(uint16_t raw);

public:
    TempHumiditySensor();
    virtual ~TempHumiditySensor() { }

    void init(DebugLog *log) {m_log = log;}

    virtual void setData(bleDeviceData_t &data) {m_data = data;}
    virtual void parse();
};

#endif // TEMP_HUMIDITY_SENSOR_H