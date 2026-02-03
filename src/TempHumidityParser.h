#ifndef TEMP_HUMIDITY_PARSER_H_
#define TEMP_HUMIDITY_PARSER_H_

#include <stdint.h>
#include <BleParser.h>

#define MAX_SEND_BUF_SIZE 128

class DebugLog;
class UploadDataClient;

class TempHumidityParser : public BleParser {
private:
    DebugLog* m_log;
    UploadDataClient* m_uploadClient;
    bleDeviceData_t m_data;

    char m_sendBuf[MAX_SEND_BUF_SIZE];

    uint8_t processBatteryVoltage(uint16_t raw);

public:
    TempHumidityParser();
    virtual ~TempHumidityParser() { }

    void init(DebugLog *log) {m_log = log;}
    void setUploadClient(UploadDataClient *client) { m_uploadClient = client; }

    virtual void setData(bleDeviceData_t &data) {m_data = data;}
    virtual void parse();
};

#endif // TEMP_HUMIDITY_PARSER_H_