#ifndef SYSTEM_STATUS_H_
#define SYSTEM_STATUS_H_

#include <stdint.h>
#include <core/Sliceable.h>
#include <core/IntervalTimer.h>

#define MAX_SYS_STAT_SEND_BUF_SIZE 128

class UploadDataClient;
class SdLogger;

class SystemStatus : public Sliceable {
private:
    static const unsigned int s_UPLOAD_TIME_MS;
    static char s_ROUTE[];

    IntervalTimer m_timer;
    UploadDataClient* m_uploadClient;
    SdLogger* m_sdLogger;
    uint8_t m_state;

    char m_sendBuf[MAX_SYS_STAT_SEND_BUF_SIZE];
    char m_logBuf[MAX_SYS_STAT_SEND_BUF_SIZE];

    void toJson(char *buf, uint32_t maxLen, bool addLineEnding);
    void uploadReadings();
    void writeReadings();

public:
    SystemStatus();
    virtual ~SystemStatus() {}
    virtual const char* sliceName( ) { return "SystemStatus"; }

    void setUploadClient(UploadDataClient *client) { m_uploadClient = client; }
    void setSdLogger(SdLogger *sdLogger) {m_sdLogger = sdLogger; }
    virtual void slice( void);

};

#endif // SYSTEM_STATUS_H_