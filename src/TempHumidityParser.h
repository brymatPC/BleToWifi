#ifndef TEMP_HUMIDITY_PARSER_H_
#define TEMP_HUMIDITY_PARSER_H_

#include <stdint.h>
#include <BleParser.h>
#include <core/Sliceable.h>
#include <core/IntervalTimer.h>

#define TEMP_HUMIDITY_MAC_LEN 6
#define MAX_TEMP_HUM_SENSORS 8
#define MAX_SEND_BUF_SIZE 128
#define MAX_ROUTE_LEN     32

class DebugLog;
class UploadDataClient;

typedef struct {
    uint8_t macAddr[TEMP_HUMIDITY_MAC_LEN];
    uint8_t batteryVoltage;
    int16_t temperature;
    int16_t humidity;
    uint32_t upTime;
} tempHumidityData_t;

class TempHumidityParser : public Sliceable, public BleParser {
private:
    static const unsigned int s_UPLOAD_TIME_MS;
    static char s_ROUTE[];

    DebugLog* m_log;
    IntervalTimer m_timer;
    UploadDataClient* m_uploadClient;
    bool m_uploadRequest;
    bleDeviceData_t m_bleData;
    uint8_t m_state;
    bool m_additionalLogging;
    uint32_t m_numDuplicates;

    tempHumidityData_t m_data[MAX_TEMP_HUM_SENSORS];
    bool m_dataFresh[MAX_TEMP_HUM_SENSORS];
    uint32_t m_lastUpdate[MAX_TEMP_HUM_SENSORS];
    char m_sendBuf[MAX_SEND_BUF_SIZE];
    uint8_t m_uploadIndex;

    uint8_t processBatteryVoltage(uint16_t raw);
    void addData(tempHumidityData_t &data);
    bool compareMacAddr(uint8_t *addr1, uint8_t *addr2);
    int8_t dataFresh(uint8_t *macAddr);

public:
    TempHumidityParser();
    virtual ~TempHumidityParser() { }
    virtual const char* sliceName( ) { return "TempHumidityParser"; }

    void init(DebugLog *log) {m_log = log;}
    void setUploadClient(UploadDataClient *client) { m_uploadClient = client; }
    virtual void slice( void);
    void enableAdditionalLogging(bool enable) { m_additionalLogging = enable; }

    virtual void setData(bleDeviceData_t &data) {m_bleData = data;}
    virtual void parse();
    virtual void scanComplete();
};

#endif // TEMP_HUMIDITY_PARSER_H_