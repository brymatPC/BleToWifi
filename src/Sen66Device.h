#ifndef SEN66_DEVICE_H
#define SEN66_DEVICE_H

#include <stdint.h>
#include <Preferences.h>
#include <core/Sliceable.h>
#include <core/IntervalTimer.h>
#include <SensirionI2cSen66.h>

#define SENSIRION_SN_LEN (32)
#define MAX_SEN66_SEND_BUF_SIZE 128

class UploadDataClient;

class Sen66Device : public Sliceable {
private:
    static const char s_PREF_NAMESPACE[];
    static const unsigned int s_SAMPLE_TIME_MS;
    static const unsigned int s_UPLOAD_TIME_MS;
    static const unsigned int s_STARTUP_OFFSET_MS;
    static char s_ROUTE[];
    
    SensirionI2cSen66 &m_sensor;
    IntervalTimer m_timer;
    IntervalTimer m_uploadTimer;
    UploadDataClient* m_uploadClient;
    bool m_uploadRequest;
    bool m_dataFresh;
    uint32_t m_lastUpdate;
    uint8_t m_state;
    uint32_t m_numDuplicates;

    int8_t m_serialNumber[SENSIRION_SN_LEN];
    uint8_t m_majorVer;
    uint8_t m_minorVer;
    uint16_t pm1p0 = 0;
    uint16_t pm2p5 = 0;
    uint16_t pm4p0 = 0;
    uint16_t pm10p0 = 0;
    int16_t humidity = 0;
    int16_t temperature = 0;
    int16_t vocIndex = 0;
    int16_t noxIndex = 0;
    uint16_t co2 = 0;
    uint32_t m_resetTimeMs = 0;

    char m_sendBuf[MAX_SEN66_SEND_BUF_SIZE];

    void read();
    void logReadings();
    void uploadReadings();

public:
    Sen66Device(SensirionI2cSen66 &sensor);
    virtual ~Sen66Device() { }
    virtual const char* sliceName( ) { return "Sen66Device"; }

    void setup(Preferences &pref);
    void save(Preferences &pref);

    void setUploadClient(UploadDataClient *client) { m_uploadClient = client; }
    virtual void slice( void);

};

#endif // SEN66_DEVICE_H