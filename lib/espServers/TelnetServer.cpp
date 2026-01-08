#include "TelnetServer.h"

#if defined (ESP32)
  #include <Wifi.h>
#elif defined (ESP8266)
  #include <ESP8266WiFi.h>
#else
  #warning "WiFi is not supported on the selected target"
#endif
#include <WiFiServer.h>
#include <WiFiClient.h>

typedef enum {
  STATE_STARTUP   = 10,
  STATE_IDLE      = 0,
  STATE_CONNECTED = 1,
  STATE_RECEIVE   = 2,
  STATE_CONVERT   = 3,

} telnetServerStates_t;

void TelnetLogServer::init( unsigned port) {
    m_fromTelnetQ = &m_fq;
    m_toTelnetQ = &m_tq;
    m_port = port;
    m_timer.setInterval( 10);
}

TelnetServer::TelnetServer() {
  m_fromTelnetQ = NULL;
  m_toTelnetQ = NULL;

  m_server = NULL;
  m_client = NULL;
  m_log = NULL;
  m_data0 = m_data1 = 0;
  m_state = STATE_STARTUP;
  m_lastCharWasNull =  m_flipFlop = m_lastConnected =  false;
}

TelnetServer::~TelnetServer() {
  if( m_server == NULL) {
    delete m_server;
    m_server = NULL;
  }
  if( m_client == NULL) {
    delete m_client;
    m_client = NULL;
  }
  m_fromTelnetQ = NULL;
  m_toTelnetQ = NULL;
  m_log = NULL;
  m_state = STATE_STARTUP;
  m_server = NULL;
  m_client = NULL;
}

void TelnetServer::init( unsigned port, CircularQBase<char> *in, CircularQBase<char>* out, DebugLog* log) {
    m_fromTelnetQ = in;
    m_toTelnetQ = out;
    m_port = port;
    m_log = log;
    m_timer.setInterval( 10);
}

void TelnetServer::changeState( uint8_t newState) {
  if( m_log != NULL) {
    m_log->print( __FILE__, __LINE__, 0x1000, (uint32_t) m_state, (uint32_t) newState, "TelnetServer_changeState: state, newState");
  }
  m_state = newState;
}

void TelnetServer::slice() {
  if( m_timer.isNextInterval()) {
    uint32_t start = HW_getMicros();
    uint8_t startState = m_state;
    uint8_t data;

    if( m_lastConnected && !m_client->connected()) {
      if( m_log != NULL) {
        m_log->print( __FILE__, __LINE__, 0x0200, "TelnetServer_client_disconnected:");
      }
      m_lastConnected = false;
      changeState( STATE_IDLE);
    } else {
      switch( m_state) {
        case STATE_STARTUP:
          // BAM - 20260107 - Need to wait for WiFi to be initialized before creating a server or client
          if(WiFi.getMode() != WIFI_MODE_NULL) {
            m_server = new WiFiServer(m_port);
            m_server->begin();
            m_client = new WiFiClient();
            changeState( STATE_IDLE);
          }
        break;
        case STATE_IDLE:
          *m_client = m_server->accept();
          if( *m_client) {
            m_lastConnected = true;
            if( m_log != NULL) {
              m_log->print( __FILE__, __LINE__, 0x0200, "TelnetServer_client_connected:");
            }
            changeState( STATE_CONNECTED);
          }
        break;
        
        case STATE_CONNECTED:
          if( m_flipFlop) {
            m_flipFlop = false;
            if( m_fromTelnetQ) {
              if( m_client->available() && m_fromTelnetQ->spaceAvailable()) {
                data =  m_client->read( );
                if( m_log != NULL) {
                  m_log->print( __FILE__, __LINE__, 0x0400, data, "TelnetServer: charReceived");
                }
                if( data != 0xFF) {
                  if( data || m_lastCharWasNull ) {
                    m_fromTelnetQ->put( data);
                    m_lastCharWasNull = false;
                  } else {
                      m_lastCharWasNull = true;
                  }
                } else {
                  m_data0 = 0xFF;
                  changeState( STATE_RECEIVE);
                }
              }
            }
          } else {
            m_flipFlop = true;
            if( m_toTelnetQ) {
              const char* p = m_toTelnetQ->getLinearReadBuffer();
              size_t len = m_toTelnetQ->getLinearReadBufferSize();
              //if( len > 0 && m_client->availableForWrite() >= (int) len) {
              if( len > 0) {
                size_t bw = m_client->write((uint8_t*) p , len );
                if( bw > 0) {
                  m_toTelnetQ->drop( bw);
                }
              }
            }
          }
        break;
        
        case STATE_RECEIVE:
          if( m_client->available() && m_fromTelnetQ->spaceAvailable()) {
            data = m_client->read();
            if( data == 0xFF) {
              m_fromTelnetQ->put( data);
              changeState( STATE_CONNECTED);
            } else {
              m_data1 = data;
              changeState( STATE_CONVERT);
            }
          }
        break;
        
        case STATE_CONVERT:
          if( m_client->available() && m_fromTelnetQ->spaceAvailable()) {
            data = m_client->read();
            if( m_log != NULL) {
              m_log->printX( __FILE__, __LINE__, 0x0800, m_data0, m_data1, data,  "TelnetServer_slice_request: m_data0, m_data1, data");
            }
            if( m_data1 == 0xFB && data == 0x03) {
              m_data1 = 0xFD;
            } else if( m_data1 == 0xFD && data == 0x03) {
              m_data1 = 0xFB;
            } else if( m_data1 == 0xFD && data == 0x01) {
              m_data1 = 0xFB;
            } else if( m_data1 == 0xFC) {
              m_data1 = 0xFE;
            } else if( m_data1 == 0xFE) {
              m_data1 = 0xFB;
            } else {
              m_data0 = m_data1 = 0;
            }
            if( m_data0) {
              m_client->write( m_data0);
              m_client->write( m_data1);
              m_client->write( data );
              if( m_log != NULL) {
                m_log->printX( __FILE__, __LINE__, 0x0800, m_data0, m_data1, data,  "TelnetServer_slice_response: m_data0, m_data1, data");
              }
              m_data0 = m_data1 = 0;
            }
            changeState( STATE_CONNECTED);
          }
        break;
      }
    }

    unsigned et =  HW_getMicros() - start;
    if(  m_log != NULL && et > 900) {
      m_log->print( __FILE__, __LINE__, 0x0100, startState, m_state, et, "TelnetServer_slice: startState, m_state, time");
    }
  }
}
