#include "UploadDataClient.h"

#include <utility/DebugLog.h>

#if defined (ESP32)
  #include <Wifi.h>
  #define WIFI_MODE_UNAVAILABLE (WIFI_MODE_NULL)
#elif defined (ESP8266)
  #include <ESP8266WiFi.h>
  #define WIFI_MODE_UNAVAILABLE (WIFI_OFF)
#else
  #warning "WiFi is not supported on the selected target"
#endif
#include <WiFiServer.h>
#include <WiFiClient.h>

typedef enum {
  STATE_STARTUP         = 0,
  STATE_RESET           = 1,
  STATE_IDLE            = 2,
  STATE_CONNECTING      = 3,
  STATE_CONNECTED       = 4,
  STATE_DISCONNECTING   = 5,
  STATE_SEND_FILE       = 6,

} ClientStates_t;

static WiFiClient s_client;

static const char* s_POST_HEADER = "POST /sensor HTTP/1.1\r\nContent-type: application/json\r\n\r\n";

UploadDataClient::UploadDataClient() {
    m_connected = false;
    m_ip[0] = '\0';
    m_port = 0;
    m_state = STATE_STARTUP;
    m_client = &s_client;
}

UploadDataClient::~UploadDataClient( void) {
}

void UploadDataClient::init(const char *ip, unsigned port, DebugLog* log) {
    strncpy(m_ip, ip, 16);
    m_port = port;
    m_log = log;
}
bool UploadDataClient::busy() {
    return m_state != STATE_IDLE && !m_sendRequest;
}
void UploadDataClient::changeState( uint8_t newState) {
    if( m_log != NULL) {
        m_log->print( __FILE__, __LINE__, 0x100001, (uint32_t) m_state, (uint32_t) newState, "UploadDataClient_changeState: state, newState");
    }
    m_state = newState;
}
void UploadDataClient::sendHeader() {
    if(m_client) {
        snprintf(m_headerBuf, MAX_HEADER_BUF_SIZE, "POST /sensor HTTP/1.1\r\nHost: %s:%d\r\nContent-type: application/json\r\nContent-Length: %d\r\n\r\n",
                WiFi.localIP().toString().c_str(), m_port, m_fileLength);
        m_client->write(m_headerBuf, strlen(m_headerBuf));
    }
}
void UploadDataClient::sendFile(char *file, unsigned len) {
    if( m_log != NULL) {
        m_log->print( __FILE__, __LINE__, 0x100001, (uint32_t) len, "UploadDataClient_sendFile: len");
    }
    if(file != nullptr && len > 0) {
        m_fileToSend = file;
        m_fileLength = len;
        m_sendRequest = true;
    }
}
void UploadDataClient::slice() {
    switch( m_state) {
        case STATE_STARTUP:
            if(m_ip[0] != '\0' && m_port != 0) {
                changeState( STATE_IDLE);
            }
        break;
        case STATE_IDLE:
            if(m_sendRequest) {
                m_sendRequest = false;
                changeState( STATE_CONNECTING);
            }
        break;
        case STATE_CONNECTING:
        {
            int ret = m_client->connect(m_ip, m_port, 100);
            if(m_log) {
                m_log->print( __FILE__, __LINE__, 0x100001, (uint32_t) ret, "UploadDataClient: connect_ret");
            }
            if(m_client->connected()) {
                changeState( STATE_CONNECTED);
            } else {
                changeState( STATE_DISCONNECTING);
            }
        }
        break;
        case STATE_CONNECTED:
            sendHeader();
            changeState( STATE_SEND_FILE);
        break;
        case STATE_SEND_FILE:
            m_client->write(m_fileToSend, m_fileLength);
            m_client->write("\r\n", 2);
            changeState( STATE_DISCONNECTING);
        break;
        case STATE_DISCONNECTING:
            m_client->stop();
            if(m_log) {
                m_log->print( __FILE__, __LINE__, 0x100001, "UploadDataClient: stopped");
            }
            changeState( STATE_IDLE);
        break;
    }
}