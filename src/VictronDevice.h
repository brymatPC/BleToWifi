#ifndef VICTRON_DEVICE_H
#define VICTRON_DEVICE_H

#include <stdint.h>
#include <BleParser.h>
#include <Preferences.h>
#include <core/Sliceable.h>
#include <core/IntervalTimer.h>

#define MAX_VIC_SEND_BUF_SIZE 128
#define VICTRON_KEY_LEN 16

class DebugLog;
class UploadDataClient;

typedef struct {
    uint16_t timeToGo;
    uint16_t batteryVoltage;
    uint32_t stateOfCharge;
    int32_t batteryCurrent;
} victronData_t;

class VictronDevice : public Sliceable, public BleParser {
private:
    static const char s_PREF_NAMESPACE[];
    static const unsigned int s_UPLOAD_TIME_MS;
    static const unsigned int s_STARTUP_OFFSET_MS;
    static char s_ROUTE[];
    
    uint8_t m_key[VICTRON_KEY_LEN];
    DebugLog* m_log;
    IntervalTimer m_timer;
    UploadDataClient* m_uploadClient;
    bool m_uploadRequest;
    bleDeviceData_t m_bleData;
    victronData_t m_data;
    bool m_dataFresh;
    uint32_t m_lastUpdate;
    uint8_t m_state;
    uint32_t m_numDuplicates;
    char m_sendBuf[MAX_VIC_SEND_BUF_SIZE];

    void decrypt();
public:
    VictronDevice();
    virtual ~VictronDevice() { }
    virtual const char* sliceName( ) { return "VictronDevice"; }

    void setup(Preferences &pref);
    void save(Preferences &pref);

    void init(DebugLog *log) {m_log = log;}
    void setUploadClient(UploadDataClient *client) { m_uploadClient = client; }
    virtual void slice( void);

    void setKey(const char *key);
    virtual void setData(bleDeviceData_t &data) {m_bleData = data;}
    virtual void parse();
    virtual void scanComplete();
};

#endif // VICTRON_DEVICE_H