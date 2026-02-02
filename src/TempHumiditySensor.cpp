#include "TempHumiditySensor.h"

#include <utility/DebugLog.h>
#include <utility/String.h>

// Credit to: https://github.com/Bluetooth-Devices/thermobeacon-ble/blob/main/src/thermobeacon_ble/parser.py

TempHumiditySensor::TempHumiditySensor() {
    m_data = bleDeviceData_t{};
}
void TempHumiditySensor::parse() {
    if(m_data.payloadLen == 0 || m_data.payload == nullptr) return;

    if(m_data.payloadLen > 7) {
        uint16_t companyId = (m_data.payload[1] << 8) | (m_data.payload[0]);

        // if(m_log) {
        //     m_log->printX( __FILE__, __LINE__, 1, companyId, m_data.payloadLen, "TempHumiditySensor: companyId, payloadLen");
        // }
    }

    // Sensor has two advertising packets, one with length 20 and one with length 22
    if(m_data.payloadLen == 20) {
        
        uint16_t batteryVoltageRaw = (m_data.payload[11] << 8) | (m_data.payload[10]);
        int16_t temperature    = (m_data.payload[13] << 8) | (m_data.payload[12]);
        int16_t humidity       = (m_data.payload[15] << 8) | (m_data.payload[14]);
        uint32_t upTime        = (m_data.payload[19] << 24) | (m_data.payload[18] << 16) | (m_data.payload[17] << 8) | (m_data.payload[16]);

        uint8_t batteryVoltage = processBatteryVoltage(batteryVoltageRaw);

        temperature = (temperature * 10) / 16;
        humidity = (humidity * 10) / 16;

        if(m_log) {
            // char outStr[128];
            // outStr[0] = '0';
            // outStr[1] = 'x';
            // for(int i=0; i < 10; i++) {
            //     sprintf(&outStr[2+i*2], "%02X", m_data.payload[i+10]);
            // }
            // m_log->print( __FILE__, __LINE__, 1, outStr, "TempHumiditySensor: outputData");
            m_log->print( __FILE__, __LINE__, 1, batteryVoltage, temperature, humidity, "TempHumiditySensor: batteryVoltage, temperature, humidity");
            m_log->print( __FILE__, __LINE__, 1, upTime, "TempHumiditySensor: upTime");
        }
    } else if(m_data.payloadLen == 22) {
        //uint32_t upTime        = (m_data.payload[21] << 24) | (m_data.payload[20] << 16) | (m_data.payload[19] << 8) | (m_data.payload[18]);
        if(m_log) {
            // char outStr[128];
            // outStr[0] = '0';
            // outStr[1] = 'x';
            // for(int i=0; i < 12; i++) {
            //     sprintf(&outStr[2+i*2], "%02X", m_data.payload[i+10]);
            // }
            //m_log->print( __FILE__, __LINE__, 1, outStr, "TempHumiditySensor: outputData22");
            m_log->print( __FILE__, __LINE__, 1, m_data.payloadLen, "TempHumiditySensor - Expected, but unknown packet: payloadLen");            
            //m_log->print( __FILE__, __LINE__, 1, upTime, "TempHumiditySensor: upTime");
        }
    } else {
        if(m_log) {
            m_log->print( __FILE__, __LINE__, 1, m_data.payloadLen, "TempHumiditySensor - insufficient bytes to parse: payloadLen");
        }
    }
}

uint8_t TempHumiditySensor::processBatteryVoltage(uint16_t raw) {
    uint8_t ret = 0;

    if(raw >= 3000) {
        ret = 100;
    } else if(raw >= 2600) {
        ret = 60 + (raw - 2600) / 10;
    } else if(raw >= 2500) {
        ret = 40 + (raw - 2500) / 5;
    } else if(raw >= 2400) {
        ret = 20 + (raw - 2400) / 5;
    } else {
        ret = 0;
    }

    return ret;
}