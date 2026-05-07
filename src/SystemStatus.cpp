#include "SystemStatus.h"
#include "UploadDataClient.h"
#include "SdLogger.h"
#include "Utilities.h"
#include "esp_log_custom.h"

typedef enum {
    STATE_RESET       = 0,
    STATE_IDLE        = 1,
    STATE_UPLOAD      = 2,
    STATE_SEND_WAIT   = 3,
    STATE_WRITE_LOG   = 4,

} sysStatusStates_t;

static const char* TAG = "SysStat";
const unsigned int SystemStatus::s_UPLOAD_TIME_MS = 60000;
char SystemStatus::s_ROUTE[] = "/system";

SystemStatus::SystemStatus() :
    m_uploadClient(nullptr),
    m_sdLogger(nullptr),
    m_state(STATE_RESET)
{
    m_timer.setInterval(s_UPLOAD_TIME_MS);
}
void SystemStatus::toJson(char *buf, uint32_t maxLen, bool addLineEnding) {
    if(!buf) return;
    char ending[3] = "\r\n";
    if(!addLineEnding) {
        ending[0] = '\0';
    }
    char ts[51];
    getRtcTimeStr(ts, 51);
    size_t minHeap = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
    uint32_t idlePercent = ulTaskGetIdleRunTimePercent();
    char mac[20];
    getEspMac(mac);
    snprintf(buf, maxLen, "{\"ts\":\"%s\",\"mac\": \"%s\",\"up\": %lu,\"mh\": %lu,\"idle\": %lu}%s",
        ts, mac, millis(), minHeap, idlePercent, ending);
}
void SystemStatus::uploadReadings() {
    if(!m_uploadClient) return;
    toJson(m_sendBuf, MAX_SYS_STAT_SEND_BUF_SIZE, false);
    m_uploadClient->sendFile(s_ROUTE, m_sendBuf, strlen(m_sendBuf));

}
void SystemStatus::writeReadings() {
    static bool firstRun = true;
    if(!m_sdLogger) return;
    toJson(m_logBuf, MAX_SYS_STAT_SEND_BUF_SIZE, true);
    m_sdLogger->log(TAG, m_logBuf, firstRun);
    firstRun = false;
}

void SystemStatus::slice( void) {
    switch(m_state) {
        case STATE_RESET:
            m_state = STATE_IDLE;
        break;
        case STATE_IDLE:
            if((m_timer.hasIntervalElapsed())) {
                m_timer.setInterval(s_UPLOAD_TIME_MS);
                ESP_LOGD(TAG, "uploading data");
                m_state = STATE_UPLOAD;
            }
        break;
        case STATE_UPLOAD:
            if(!m_uploadClient) {
                m_state = STATE_WRITE_LOG;
            } else if(!m_uploadClient->busy()) {
                uploadReadings();
                m_state = STATE_SEND_WAIT;
            }
        break;
        case STATE_SEND_WAIT:
            if(!m_uploadClient->busy()) {
                m_state = STATE_WRITE_LOG;
            }
        break;
        case STATE_WRITE_LOG:
            if(m_sdLogger) {
                writeReadings();
            }
            m_state = STATE_IDLE;
        break;
        default:
            m_state = STATE_RESET;
        break;
    }
}