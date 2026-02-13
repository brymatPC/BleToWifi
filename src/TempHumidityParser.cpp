#include "TempHumidityParser.h"
#include "UploadDataClient.h"

// Credit to: https://github.com/Bluetooth-Devices/thermobeacon-ble/blob/main/src/thermobeacon_ble/parser.py

typedef enum {
  STATE_RESET       = 0,
  STATE_IDLE        = 1,
  STATE_UPLOAD      = 2,
  STATE_UPLOAD_WAIT = 3,
  STATE_SEND_WAIT   = 4,

} tempHumStates_t;

static const char* TAG = "THParse";

const unsigned int TempHumidityParser::s_UPLOAD_TIME_MS = 120000;
char TempHumidityParser::s_ROUTE[] = "/sensor";

TempHumidityParser::TempHumidityParser() {
    m_bleData = bleDeviceData_t{};
    for(uint8_t i=0; i < MAX_TEMP_HUM_SENSORS; i++) {
        m_dataFresh[i] = false;
        m_lastUpdate[i] = 0;
        memset(m_data[i].macAddr, 0, TEMP_HUMIDITY_MAC_LEN);
    }
    m_state = STATE_RESET;
    m_additionalLogging = false;
    m_timer.setInterval(s_UPLOAD_TIME_MS);
    m_uploadRequest = false;
    m_numDuplicates = 0;
}
void TempHumidityParser::parse() {
    if(m_bleData.payloadLen == 0 || m_bleData.payload == nullptr) return;

    if(m_bleData.payloadLen > 7) {
        uint16_t companyId = (m_bleData.payload[1] << 8) | (m_bleData.payload[0]);

        if(m_additionalLogging) {
            char outStr[20];
            outStr[0] = '0';
            outStr[1] = 'x';
            for(int i=0; i < TEMP_HUMIDITY_MAC_LEN; i++) {
                sprintf(&outStr[2+i*2], "%02X", m_bleData.payload[i+4]);
            }
            ESP_LOGI(TAG, "macAddr=%s", outStr);
            ESP_LOGI(TAG, "companyId=0x%04X, payloadLen=%lu", companyId, m_bleData.payloadLen);
        }
    }

    // Sensor has two advertising packets, one with length 20 and one with length 22
    if(m_bleData.payloadLen == 20) {
        int8_t index = dataFresh(&m_bleData.payload[4]);
        if(index >= 0 && (millis() < (m_lastUpdate[index] + 30000))) {
            ESP_LOGD(TAG, "Probable duplicate: index=%u millis=%u m_lastUpdate=%u", index, (unsigned)millis(), (unsigned)m_lastUpdate);
            m_numDuplicates++;
            return;
        }
        tempHumidityData_t temp;
        memcpy(temp.macAddr, &m_bleData.payload[4], TEMP_HUMIDITY_MAC_LEN);
        uint16_t batteryVoltageRaw = (m_bleData.payload[11] << 8) | (m_bleData.payload[10]);
        temp.temperature    = (m_bleData.payload[13] << 8) | (m_bleData.payload[12]);
        temp.humidity       = (m_bleData.payload[15] << 8) | (m_bleData.payload[14]);
        temp.upTime        = (m_bleData.payload[19] << 24) | (m_bleData.payload[18] << 16) | (m_bleData.payload[17] << 8) | (m_bleData.payload[16]);

        temp.batteryVoltage = processBatteryVoltage(batteryVoltageRaw);

        temp.temperature = (temp.temperature * 10) / 16;
        temp.humidity = (temp.humidity * 10) / 16;

        if(m_additionalLogging) {
            char outStr[128];
            outStr[0] = '0';
            outStr[1] = 'x';
            for(int i=0; i < 10; i++) {
                sprintf(&outStr[2+i*2], "%02X", m_bleData.payload[i+10]);
            }
            ESP_LOGI(TAG, "outputData=%s", outStr);
        }
        ESP_LOGI(TAG, "addr=%s", m_bleData.addr);
        ESP_LOGI(TAG, "temperature=%d humidity=%d", temp.temperature, temp.humidity);

        //m_log->print( __FILE__, __LINE__, 1, temp.batteryVoltage, temp.temperature, temp.humidity, "TempHumidityParser: batteryVoltage, temperature, humidity");
        //m_log->print( __FILE__, __LINE__, 1, temp.upTime, "TempHumidityParser: upTime");

        addData(temp);

    } else if(m_bleData.payloadLen == 22) {
        int8_t index = dataFresh(&m_bleData.payload[4]);
        if(index >= 0) {
            ESP_LOGD(TAG, "Probable duplicate: index=%u millis=%u m_lastUpdate=%u", index, (unsigned)millis(), (unsigned)m_lastUpdate);
            m_numDuplicates++;
            return;
        }
        int16_t highestTemp = (m_bleData.payload[11] << 8) | (m_bleData.payload[10]);
        uint32_t highestTempRuntime = (m_bleData.payload[15] << 24) | (m_bleData.payload[14] << 16) | (m_bleData.payload[13] << 8) | (m_bleData.payload[12]);
        int16_t lowestTemp = (m_bleData.payload[17] << 8) | (m_bleData.payload[16]);
        uint32_t lowestTempRuntime = (m_bleData.payload[21] << 24) | (m_bleData.payload[20] << 16) | (m_bleData.payload[19] << 8) | (m_bleData.payload[18]);

        highestTemp = (highestTemp * 10) / 16;
        lowestTemp = (lowestTemp * 10) / 16;

        if(m_additionalLogging) {
            char outStr[128];
            outStr[0] = '0';
            outStr[1] = 'x';
            for(int i=0; i < 12; i++) {
                sprintf(&outStr[2+i*2], "%02X", m_bleData.payload[i+10]);
            }
            ESP_LOGI(TAG, "outputData22=%s", outStr);
        }
        ESP_LOGI(TAG, "addr=%s", m_bleData.addr);
        ESP_LOGI(TAG, "highestTemp=%d runTime=%lu", highestTemp, highestTempRuntime);
        ESP_LOGI(TAG, "lowestTemp=%d runTime=%lu", lowestTemp, lowestTempRuntime);
    } else {
        ESP_LOGW(TAG, "insufficient bytes to parse: payloadLen=%u", m_bleData.payloadLen);
    }
}

uint8_t TempHumidityParser::processBatteryVoltage(uint16_t raw) {
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
bool TempHumidityParser::compareMacAddr(uint8_t *addr1, uint8_t *addr2) {
    bool ret = true;
    for(uint8_t i=0; i < TEMP_HUMIDITY_MAC_LEN; i++) {
        if(addr1[i] != addr2[i]) {
            ret = false;
            break;
        }
    }
    return ret;
}
int8_t TempHumidityParser::dataFresh(uint8_t *macAddr) {
    int8_t ret = -1;
    for(uint8_t i=0; i < MAX_TEMP_HUM_SENSORS; i++) {
        if(compareMacAddr(m_data[i].macAddr, macAddr) && m_dataFresh[i]) {
            return (int8_t) i;
        }
    }
    return ret;
}
void TempHumidityParser::addData(tempHumidityData_t &data) {
    int8_t index = -1;
    int8_t emptyIndex = -1;
    for(uint8_t i=0; i < MAX_TEMP_HUM_SENSORS; i++) {
        if(compareMacAddr(m_data[i].macAddr, data.macAddr)) {
            index = (int8_t) i;
            break;
        }else if(emptyIndex == -1 && !m_dataFresh[i]) {
            emptyIndex = (int8_t) i;
            break;
        }
    }
    if(index < 0 && emptyIndex >= 0) {
        index = emptyIndex;
    }
    if(index >= 0) {
        m_data[index] = data;
        ESP_LOGI(TAG, "Found index: %u, %u", index, m_dataFresh[index]);
        m_dataFresh[index] = true;
        m_lastUpdate[index] = millis();
    } else {
        ESP_LOGW(TAG, "Failed to find empty index");
    }
}

void TempHumidityParser::scanComplete() {
    m_uploadRequest = true;
    ESP_LOGI(TAG, "ScanComplete: m_numDuplicates=%u", m_numDuplicates);
    m_numDuplicates = 0;
}
void TempHumidityParser::slice( void) {
    switch(m_state) {
        case STATE_RESET:
            m_state = STATE_IDLE;
        break;
        case STATE_IDLE:
            if((m_timer.hasIntervalElapsed() || m_uploadRequest) && m_uploadClient) {
                m_timer.setInterval(s_UPLOAD_TIME_MS);
                ESP_LOGD(TAG, "uploading data");
                m_uploadRequest = false;
                m_uploadIndex = 0;
                m_state = STATE_UPLOAD;
            }
        break;
        case STATE_UPLOAD:
            if(m_uploadIndex >= MAX_TEMP_HUM_SENSORS) {
                ESP_LOGD(TAG, "upload complete");
                m_state = STATE_IDLE;
            } else if(m_dataFresh[m_uploadIndex]) {
                m_state = STATE_UPLOAD_WAIT;
            } else {
                m_uploadIndex++;
            }
        break;
        case STATE_UPLOAD_WAIT:
            if(!m_uploadClient->busy()) {
                snprintf(m_sendBuf, MAX_SEND_BUF_SIZE, "{\"sn\":\"%02X%02X%02X%02X%02X%02X\",\"v\":%d,\"t\":%d,\"h\":%d,\"ut\":%d}",
                    m_data[m_uploadIndex].macAddr[0], m_data[m_uploadIndex].macAddr[1], m_data[m_uploadIndex].macAddr[2], m_data[m_uploadIndex].macAddr[3],
                    m_data[m_uploadIndex].macAddr[4], m_data[m_uploadIndex].macAddr[5],
                    m_data[m_uploadIndex].batteryVoltage, m_data[m_uploadIndex].temperature, m_data[m_uploadIndex].humidity, m_data[m_uploadIndex].upTime);
                m_uploadClient->sendFile(s_ROUTE, m_sendBuf, strlen(m_sendBuf));
                m_dataFresh[m_uploadIndex] = false;
                m_state = STATE_SEND_WAIT;
            }
        break;
        case STATE_SEND_WAIT:
            if(!m_uploadClient->busy()) {
                m_uploadIndex++;
                m_state = STATE_UPLOAD;
            }
        break;
        default:
            m_state = STATE_RESET;
        break;
    }
}