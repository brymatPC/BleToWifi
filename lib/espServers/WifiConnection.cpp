#include "WifiConnection.h"
#include <utility/String.h>

#define BLINK_SPEED_CONNECTING_MS 200
#define BLINK_SPEED_SCANNING_MS 400

const char WifiConnection::s_PREF_NAMESPACE[] = "wifi";
const char WifiConnection::s_DEFAULT_HOST_NAME[] = "esp32";
const char WifiConnection::s_DEFAULT_HOST_PASSWORD[] = "espPassword";
// 192.168.10.2
const char WifiConnection::s_DEFAULT_HOST_IP[] = "0x020AA8C0";
// 192.168.10.1
const char WifiConnection::s_DEFAULT_HOST_GATEWAY[] = "0x010AA8C0";
const char WifiConnection::s_DEFAULT_HOST_MASK[] = "0x00FFFFFF";

typedef enum {
  STATE_RESET             = 0,
  STATE_INIT_CONNECT      = 1,
  STATE_LOAD_NETWORK_NAME = 2,
  STATE_WAIT_CONNECT      = 3,
  STATE_NEXT_NETWORK      = 4,
  STATE_CONNECTING        = 5,
  STATE_CONNECTED         = 6,
  STATE_INIT_AP           = 7,
  STATE_NETWORK_SCAN      = 8,
  STATE_PARSE_NETWORKS    = 9,
  STATE_CONFIGURE_AP      = 10,
  STATE_AP_READY          = 11,
  STATE_OFF               = 12,

} wifiStates_t;

WifiConnection::WifiConnection( LedDriver* led, DebugLog* log, uint32_t connectTimeout) {
  m_led = led;
  m_log = log;
  m_connectTimeout = connectTimeout;
  m_currentAp = 0;
  m_state = STATE_RESET;
  m_enable = false;
  m_maxRssi = -1024;
  m_hostActive = false;

  m_hostName[0] = '\0';
  m_hostPassword[0] = '\0';
  m_hostIp[0] = '\0';
  m_hostGateway[0] = '\0';
  m_hostMask[0] = '\0';

  for(uint8_t i=0; i < MAX_WIFI_NETWORKS; i++) {
    m_networkName[i][0] = '\0';
    m_networkPassword[i][0] = '\0';
  }

  m_networkIp = 0;
}

void WifiConnection::setup(Preferences &pref) {
    char parserKey[16];
    pref.begin(s_PREF_NAMESPACE, true);
    if(pref.isKey("hName")) {
        pref.getString("hName", m_hostName, MAX_WIFI_ENTRY_LEN);
    } else {
        strcpy(m_hostName, s_DEFAULT_HOST_NAME);
    }
    if(pref.isKey("hPass")) {
        pref.getString("hPass", m_hostPassword, MAX_WIFI_ENTRY_LEN);
    } else {
        strcpy(m_hostPassword, s_DEFAULT_HOST_PASSWORD);
    }
    if(pref.isKey("hIp")) {
        pref.getString("hIp", m_hostIp, MAX_WIFI_ENTRY_LEN);
    } else {
        strcpy(m_hostIp, s_DEFAULT_HOST_IP);
    }
    if(pref.isKey("hGate")) {
        pref.getString("hGate", m_hostGateway, MAX_WIFI_ENTRY_LEN);
    } else {
        strcpy(m_hostGateway, s_DEFAULT_HOST_GATEWAY);
    }
    if(pref.isKey("hMask")) {
        pref.getString("hMask", m_hostMask, MAX_WIFI_ENTRY_LEN);
    } else {
        strcpy(m_hostMask, s_DEFAULT_HOST_MASK);
    }
    for(uint8_t i=0; i < MAX_WIFI_NETWORKS; i++) {
        snprintf(parserKey, 16, "nName%d", i);
        pref.getString(parserKey, m_networkName[i], MAX_WIFI_ENTRY_LEN);
        snprintf(parserKey, 16, "nPass%d", i);
        pref.getString(parserKey, m_networkPassword[i], MAX_WIFI_ENTRY_LEN);
    }
    pref.end();
}
void WifiConnection::save(Preferences &pref) {
    char parserKey[16];
    pref.begin(s_PREF_NAMESPACE, false);
    pref.putString("hName", m_hostName);
    pref.putString("hPass", m_hostPassword);
    pref.putString("hIp", m_hostIp);
    pref.putString("hGate", m_hostGateway);
    pref.putString("hMask", m_hostMask);
    for(uint8_t i=0; i < MAX_WIFI_NETWORKS; i++) {
        snprintf(parserKey, 16, "nName%d", i);
        pref.putString(parserKey, m_networkName[i]);
        snprintf(parserKey, 16, "nPass%d", i);
        pref.putString(parserKey, m_networkPassword[i]);
    }
    pref.end();
    if( m_log) {
        m_log->print( __FILE__, __LINE__, 1, "WifiConnection::save: pref updated" );
    }
}
const char* WifiConnection::getNetworkIp( void) {
    static char ipStr[MAX_WIFI_ENTRY_LEN];
    snprintf(ipStr, MAX_WIFI_ENTRY_LEN, "0x%08X", m_networkIp);
    return ipStr;
}
const char* WifiConnection::getNetworkName( uint8_t index) {
    if(index >= MAX_WIFI_NETWORKS) return nullptr;
    return m_networkName[index];
}
const char* WifiConnection::getNetworkPassword( uint8_t index) {
    if(index >= MAX_WIFI_NETWORKS) return nullptr;
    return m_networkPassword[index];
}
void WifiConnection::setHostName( const char* networkName) {
    strncpy(m_hostName, networkName, MAX_WIFI_ENTRY_LEN);
}
void WifiConnection::setHostPassword( const char* networkPassword ) {
    strncpy(m_hostPassword, networkPassword, MAX_WIFI_ENTRY_LEN);
}
void WifiConnection::setHostIp( const char* ip ) {
    strncpy(m_hostIp, ip, MAX_WIFI_ENTRY_LEN);
}
void WifiConnection::setHostGateway( const char* gateway) {
    strncpy(m_hostGateway, gateway, MAX_WIFI_ENTRY_LEN);
}
void WifiConnection::setHostMask( const char* mask) {
    strncpy(m_hostMask, mask, MAX_WIFI_ENTRY_LEN);
}
void WifiConnection::setNetworkName( uint8_t index, const char* networkName) {
    if(index >= MAX_WIFI_NETWORKS) return;
    strncpy(m_networkName[index], networkName, MAX_WIFI_ENTRY_LEN);
}
void WifiConnection::setNetworkPassword( uint8_t index, const char* networkPassword) {
    if(index >= MAX_WIFI_NETWORKS) return;
    strncpy(m_networkPassword[index], networkPassword, MAX_WIFI_ENTRY_LEN);
}
void WifiConnection::changeState( uint8_t state) {
  if( m_log) {
    m_log->print( __FILE__, __LINE__, 1, m_state, state, "WifiConnection::changeState: m_state, state" );
  }
  m_state = state;
}

int WifiConnection::getConnectedNetworkIndex( void) {
  return isNetworkConnected() ? m_currentAp : -1;
}

bool WifiConnection::isNetworkConnected( void) {
  return WiFi.status() == WL_CONNECTED;
}

void WifiConnection::slice( ) {
  const char* p;
  const char* q;
  int i, k, m;
  switch( m_state) {
    case STATE_RESET:
      m_currentAp = 0;
      if( m_led) {
        m_led->off();
      }
      changeState( STATE_INIT_AP);
    break;

    case STATE_INIT_CONNECT:
      if( m_led) {
        m_led->off();
      }
      if( m_enable && getNumberOfNetworks() >= 0) {
        m_timer.setInterval( m_connectTimeout);
        changeState( STATE_LOAD_NETWORK_NAME);
      }
    break;

    case STATE_LOAD_NETWORK_NAME:
      if( m_led) {
        m_led->blink( BLINK_SPEED_CONNECTING_MS);
      }
      p = getNetworkName( m_currentAp );
      q = getNetworkPassword( m_currentAp );
      m_log->print( __FILE__, __LINE__, 1, m_currentAp, "WifiConnection::slice_connecting: currentAp" );
      m_log->print( __FILE__, __LINE__, 1, p, q, "WifiConnection::slice_connecting: p, q" );
      if( *p == '\0' || *q == '\0') {
        changeState( STATE_WAIT_CONNECT);
      } else {
        WiFi.begin( (char*)p,  q);
        changeState( STATE_WAIT_CONNECT);
        if( m_log) {
          m_log->print( __FILE__, __LINE__, 1, p, "WifiConnection::slice_connecting: networkName" );
        }
      }
    break;

    case STATE_WAIT_CONNECT:
      if( WiFi.status() == WL_CONNECTED) {
        m_networkIp = (uint32_t) WiFi.localIP();
        changeState( STATE_CONNECTING);
      } else if( m_timer.hasIntervalElapsed()) {
        changeState( STATE_NEXT_NETWORK);
      } 
    break;

    case STATE_NEXT_NETWORK:
      m_currentAp++;
      if( m_currentAp >= getNumberOfNetworks() ) {
        m_currentAp = 0;
      }
      changeState( STATE_INIT_CONNECT);
    break;

    case STATE_CONNECTING:
      p = getNetworkName( m_currentAp );
      if( m_log) {
        m_log->print( __FILE__, __LINE__, 1,  p, "WifiConnection::slice_connected: networkName" );
      }
      m_timer.setInterval( 500);
      changeState( STATE_CONNECTED);
      if( m_led) {
        m_led->on();
      }
    break;

    case STATE_CONNECTED:
      if( m_timer.isNextInterval() ) {
        if(WiFi.status() != WL_CONNECTED) {
            if( m_log) {
              m_log->print( __FILE__, __LINE__, 1,  getNetworkName( m_currentAp ), "WifiConnection::slice_disconnected: networkName" );
            }
          changeState( STATE_NEXT_NETWORK);
        }
      }
    break;

    case STATE_INIT_AP:
      WiFi.mode(WIFI_STA);
      WiFi.disconnect(  );
      m_timer.setInterval( 100);
      changeState( STATE_NETWORK_SCAN);
      m_maxRssi = -1024;
      m_maxRssiIndex = 0;
      if( m_led) {
        m_led->blink( BLINK_SPEED_SCANNING_MS);
      }

    break;

    case STATE_NETWORK_SCAN:
      if( m_timer.hasIntervalElapsed() ) {
        WiFi.scanNetworks( true);
        m_timer.setInterval( m_connectTimeout * 2);
        if( m_log) {
          m_log->print( __FILE__, __LINE__, 1, "WifiConnection_slice_scanStarted: " );
        }
        changeState( STATE_PARSE_NETWORKS);
      }
    break;

    case STATE_PARSE_NETWORKS:
      i = WiFi.scanComplete();
      if( i > 0) {
        m = getNumberOfNetworks();
        if( m_log) {
          m_log->print( __FILE__, __LINE__, 1, i, m, "WifiConnection_slice_scan: numberOfNetworks, lastIndex" );
        }
        for( int j = 0; j < i; j++ ) {
          for( k = 0; k < m; k++ ) {
            if( !strcmp( WiFi.SSID( j).c_str(), getNetworkName( k)) ) {
              int32_t RSSI;
              RSSI = WiFi.RSSI( j);
              if( RSSI > m_maxRssi) {
                m_maxRssi = RSSI;
                m_maxRssiIndex = k;
              }
              if( m_log) {
                m_log->print( __FILE__, __LINE__, 1, m_maxRssiIndex, 0 - m_maxRssi, WiFi.SSID( j).c_str(), "WifiConnection::slice_scan: maxRssiIndex, maxRssi, networkName");
              }
            }
          }
        }
        WiFi.scanDelete( );
        changeState( STATE_CONFIGURE_AP);
      } else if( m_timer.hasIntervalElapsed()) {
        if( m_log) {
          m_log->print( __FILE__, __LINE__, 1, i, "WifiConnection::slice_scan_timeout:" );
        }
        WiFi.scanDelete( );
        changeState( STATE_CONFIGURE_AP);
      }
    break;

    case STATE_CONFIGURE_AP:
      p = m_hostName;
      q = m_hostPassword;
      m_log->print( __FILE__, __LINE__, 1, p, q, "WifiConnection::slice_host: p, q" );
      if( *p == '\0' || *q == '\0') {
        if( m_log) {
          m_log->print( __FILE__, __LINE__, 1, p, q, "WifiConnection::slice_no_host_notConnecting: p, q" );
        }
      } else {
        hostConfig( );
        if( WiFi.softAP( p, q) ) {
            m_hostActive = true;
          if( m_log) {
            m_log->print( __FILE__, __LINE__, 1, p, q, "WifiConnection_slice_host_up: networkName, networkPassword" );
            m_log->print( __FILE__, __LINE__, 1, WiFi.softAPIP().toString().c_str(), WiFi.softAPmacAddress().c_str(), "WifiConnection_slice_host_up: softApIp, softApMac" );
          }
        } else {
          m_hostActive = false;
          if( m_log) {
            m_log->print( __FILE__, __LINE__, 1, p, q, "WifiConnection::slice_host_not_up: p, q" );
          }
        }
      }
      changeState( STATE_AP_READY);
    break;

    case STATE_AP_READY:
      m_currentAp = m_maxRssiIndex == 0 ? 0 : m_maxRssiIndex;
      m_log->print( __FILE__, __LINE__, 1, m_enable,  m_currentAp, getNumberOfNetworks(), "WifiConnection::slice_scan_done: enable, m_currentAp, numberOfNetworks" );
      changeState( STATE_INIT_CONNECT);
      if( m_led) {
        m_led->off();
      }
    break;
    case STATE_OFF:
        // Wait for reboot or wake up
    break;
    default:
        if( m_log) {
            m_log->print( __FILE__, __LINE__, 1, m_state, "WifiConnection: invalid state");
        }
        changeState( STATE_RESET);
    break;
  }
}
void WifiConnection::off() {
    if(WiFi.isConnected()) {
        WiFi.disconnect();
    }
    WiFi.softAPdisconnect();

    changeState( STATE_OFF);
}
void WifiConnection::hostConfig( ) {
  uint32_t v;
  uint32_t ip = 0;
  uint32_t gw = 0;
  uint32_t mask = 0;

  if( stringToUnsignedX( m_hostIp, &v)) {
    ip = v;
  }
  if( stringToUnsignedX( m_hostGateway, &v)) {
    gw = v;
  }
  if( stringToUnsignedX( m_hostMask, &v)) {
    mask = v;
  }
  if( !WiFi.softAPConfig( IPAddress(ip), IPAddress(gw) , IPAddress( mask))) {
    if( m_log) {
      m_log->print( __FILE__, __LINE__, 1, "WifiConnection_hostConfig_failed" );
    }
  }
}
