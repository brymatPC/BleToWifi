#include "UploadDataClient.h"

#include <utility/DebugLog.h>

#if defined (ESP32)
  #include <Wifi.h>
  #define WIFI_MODE_UNAVAILABLE (WIFI_MODE_NULL)
#else
  #warning "WiFi is not supported on the selected target"
#endif

#include <NetworkClient.h>

typedef enum {
  STATE_STARTUP         = 0,
  STATE_RESET           = 1,
  STATE_IDLE            = 2,
  STATE_CONNECTING      = 3,
  STATE_CONNECTED       = 4,
  STATE_DISCONNECTING   = 5,
  STATE_SEND_FILE       = 6,

} ClientStates_t;

static NetworkClient s_client;

const char UploadDataClient::s_PREF_NAMESPACE[] = "udc";

UploadDataClient::UploadDataClient() {
    m_connected = false;
    m_ip[0] = '\0';
    m_port = 0;
    m_state = STATE_STARTUP;
    m_client = &s_client;
}

UploadDataClient::~UploadDataClient( void) {
}

void UploadDataClient::init(DebugLog* log) {
    m_log = log;
}
void UploadDataClient::setup(Preferences &pref) {
    pref.begin(s_PREF_NAMESPACE, true);
    pref.getString("ip", m_ip, UDC_IP_LEN);
    m_port = pref.getULong("port", 0);
    pref.end();
}
void UploadDataClient::save(Preferences &pref) {
    pref.begin(s_PREF_NAMESPACE, false);
    pref.putString("ip", m_ip);
    pref.putULong("port", m_port);
    pref.end();
    if( m_log) {
        m_log->print( __FILE__, __LINE__, 1, "UploadDataClient::save: pref updated" );
    }
}
void UploadDataClient::setHostIp(const char *ip) {
    strncpy(m_ip, ip, UDC_IP_LEN);
}
void UploadDataClient::setHostPort(unsigned port) {
    m_port = port;
}
bool UploadDataClient::busy() {
    return m_state != STATE_IDLE && !m_sendRequest;
}
void UploadDataClient::changeState( uint8_t newState) {
    if( m_log != NULL) {
        m_log->print( __FILE__, __LINE__, 0x100000, (uint32_t) m_state, (uint32_t) newState, "UploadDataClient_changeState: state, newState");
    }
    m_state = newState;
}
void UploadDataClient::sendHeader() {
    if(m_client) {
        snprintf(m_headerBuf, MAX_HEADER_BUF_SIZE, "POST %s HTTP/1.1\r\nHost: %s:%d\r\nContent-type: application/json\r\nContent-Length: %d\r\n\r\n",
                m_routeToSend, WiFi.localIP().toString().c_str(), m_port, m_fileLength);
        m_client->write(m_headerBuf, strlen(m_headerBuf));
    }
}
void UploadDataClient::sendFile(char *route, char *file, unsigned len) {
    if( m_log != NULL) {
        m_log->print( __FILE__, __LINE__, 0x100001, (uint32_t) len, "UploadDataClient_sendFile: len");
    }
    if(route != nullptr && file != nullptr && len > 0) {
        m_routeToSend = route;
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
                m_log->print( __FILE__, __LINE__, 0x100001, "UploadDataClient: done");
            }
            changeState( STATE_IDLE);
        break;
    }
}