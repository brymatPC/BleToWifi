#include "Sen66Device.h"
#include "UploadDataClient.h"
#include <esp_log.h>

// Used by Sensirion Library
#ifndef NO_ERROR
#define NO_ERROR 0
#endif

typedef enum {
  STATE_RESET       = 0,
  STATE_RESET_WAIT  = 1,
  STATE_START       = 2,
  STATE_IDLE        = 3,
  STATE_UPLOAD      = 4,
  STATE_UPLOAD_WAIT = 5,
  STATE_SEND_WAIT   = 6,
  STATE_READ        = 7,
  STATE_ERROR       = 10,
  STATE_ERROR_WAIT  = 11,

} sen66States_t;

static const char* TAG = "Sen66";

const char Sen66Device::s_PREF_NAMESPACE[] = "vic";
const unsigned int Sen66Device::s_SAMPLE_TIME_MS = 10000;
const unsigned int Sen66Device::s_UPLOAD_TIME_MS = 120000;
const unsigned int Sen66Device::s_STARTUP_OFFSET_MS = 0;
char Sen66Device::s_ROUTE[] = "/sen66";

Sen66Device::Sen66Device(SensirionI2cSen66 &sensor) :
    m_sensor(sensor)
{
    m_dataFresh = false;
    m_lastUpdate = 0;
    m_state = STATE_RESET;
    m_uploadTimer.setInterval(s_STARTUP_OFFSET_MS);
    m_uploadRequest = false;
    m_numDuplicates = 0;
}
void Sen66Device::setup(Preferences &pref) {
    pref.begin(s_PREF_NAMESPACE, true);
    
    pref.end();
}
void Sen66Device::save(Preferences &pref) {
    char parserKey[16];
    pref.begin(s_PREF_NAMESPACE, false);
    
    pref.end();
    ESP_LOGI(TAG, "pref updated");
}
void Sen66Device::read() {

    int16_t error = m_sensor.readMeasuredValuesAsIntegers(
        pm1p0, pm2p5, pm4p0, pm10p0, humidity, temperature, vocIndex, noxIndex, co2);
    if (error != NO_ERROR) {
        ESP_LOGW(TAG, "error executing read_measured_values_as_integers(): %i", error);
    } else {
        ESP_LOGI(TAG, "pm1p0: %u, pm2p5: %u, pm4p0: %u, pm10p0: %u", pm1p0, pm2p5, pm4p0, pm10p0);
        ESP_LOGI(TAG, "temperature: %i, humidity: %i", temperature, humidity);
        ESP_LOGI(TAG, "vocIndex: %i, noxIndex: %i, co2: %u", vocIndex, noxIndex, co2);
    }
}
void Sen66Device::slice( void) {
    int16_t error;
    switch(m_state) {
        case STATE_RESET:
            error = m_sensor.deviceReset();
            if (error != NO_ERROR) {
                ESP_LOGW(TAG, "error executing device_reset(): %i", error);
                m_state = STATE_ERROR;
            } else {
                m_timer.setInterval(1200);
                m_state = STATE_RESET_WAIT;
            }
        break;
        case STATE_RESET_WAIT:
            if(m_timer.hasIntervalElapsed()) {
                ESP_LOGI(TAG, "Starting sensiron");
                m_state = STATE_START;
            }
        break;
        case STATE_START:
            error = m_sensor.getSerialNumber(m_serialNumber, SENSIRION_SN_LEN);
            if (error != NO_ERROR) {
                ESP_LOGW(TAG, "error executing get_serial_number(): %i", error);
                m_state = STATE_ERROR;
            } else {
                ESP_LOGI(TAG, "serial_number: %s", m_serialNumber);
                error = m_sensor.startContinuousMeasurement();
                if (error != NO_ERROR) {
                    ESP_LOGW(TAG, "error executing start_continuous_measurement(): %i", error);
                    m_state = STATE_ERROR;
                } else {
                    m_timer.setInterval(s_SAMPLE_TIME_MS);
                    m_uploadTimer.setInterval(s_UPLOAD_TIME_MS);
                    m_state = STATE_IDLE;
                }
            }
        break;
        case STATE_IDLE:
            if(m_timer.isNextInterval()) {
                m_state = STATE_READ;
            } else if((m_uploadTimer.hasIntervalElapsed()) && m_uploadClient) {
                m_uploadTimer.setInterval(s_UPLOAD_TIME_MS);
                ESP_LOGD(TAG, "uploading data");
                m_uploadRequest = false;
                m_state = STATE_UPLOAD;
            }
        break;
        case STATE_READ:
            read();
            m_state = STATE_IDLE;
        break;
        case STATE_UPLOAD:
            if(m_dataFresh) {
                m_state = STATE_UPLOAD_WAIT;
            } else {
                ESP_LOGD(TAG, "No data to upload");
                m_state = STATE_IDLE;
            }
        break;
        case STATE_UPLOAD_WAIT:
            if(!m_uploadClient->busy()) {
                // TODO: Add data upload
                // snprintf(m_sendBuf, MAX_VIC_SEND_BUF_SIZE, "{\"ttg\":%d,\"v\":%d,\"i\":%d,\"soc\":%d}",
                //     m_data.timeToGo, m_data.batteryVoltage, m_data.batteryCurrent, m_data.stateOfCharge);
                // m_uploadClient->sendFile(s_ROUTE, m_sendBuf, strlen(m_sendBuf));
                m_dataFresh = false;
                m_state = STATE_SEND_WAIT;
            }
        break;
        case STATE_SEND_WAIT:
            if(!m_uploadClient->busy()) {
                ESP_LOGD(TAG, "upload complete");
                m_state = STATE_IDLE;
            }
        break;
        case STATE_ERROR:
            m_timer.setInterval(s_SAMPLE_TIME_MS);
            m_state = STATE_ERROR_WAIT;
        break;
        case STATE_ERROR_WAIT:
            if(m_timer.hasIntervalElapsed()) {
                ESP_LOGI(TAG, "Sensiron retry");
                m_state = STATE_RESET;
            }
        break;
        default:
            m_state = STATE_RESET;
        break;
    }
}