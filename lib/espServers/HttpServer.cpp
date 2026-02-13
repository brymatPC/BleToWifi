#include "HttpServer.h"

#if defined (ESP32)
  #include <Wifi.h>
  #define WIFI_MODE_UNAVAILABLE (WIFI_MODE_NULL)
#else
  #warning "WiFi is not supported on the selected target"
#endif
#include <NetworkServer.h>
#include <NetworkClient.h>

typedef enum {
  STATE_STARTUP          = 30,
  STATE_RESET            = 0,
  STATE_IDLE            = 1,
  STATE_CONNECTING      = 20,
  STATE_CONNECTED       = 21,
  STATE_DISCONNECTING   = 3,
  STATE_PROCESS_REQUEST = 2,
  STATE_LOG_DISCONNECT  = 4,
  STATE_DISCONNECT_WAIT = 5,
  STATE_SEND_FILE       = 6,
  STATE_PROCESS_EXEC    = 7,
  STATE_FINISH_EXEC     = 8,
  STATE_PROCESS_CMD     = 9,
  STATE_FINISH_CMD      = 10,

} httpServerStates_t;

static const char* TAG = "HttpS";

static char charToHex( char c) {
    char value = '\0';
    if(  c >= '0' && c <= '9' ) {
        value |= c - '0';
    } else if(  c >= 'a' && c <= 'f' ) {
        value |= c - 'a' + 10;
    } else if(  c >= 'A' && c <= 'F') {
        value |= c - 'A' + 10;
    }
    return value;
}

HttpServer::HttpServer( void) {
  m_server = NULL;
  m_client = NULL;

  m_port = 0;
  m_state = STATE_STARTUP;
  m_urlIndex = 0;
}

HttpServer::~HttpServer( void) {
  if( m_server != NULL) {
    delete m_server;
    m_server = NULL;
  }
  if( m_client != NULL) {
    delete m_client;
    m_client = NULL;
  }
  m_state = STATE_STARTUP;
  m_responseCode = 0;
  m_urlIndex = 0;
}

void HttpServer::init(unsigned port) {
    m_port = port;
    m_client = NULL;
    m_state = STATE_STARTUP;
    m_urlIndex = 0;
}
unsigned HttpServer::readFile( char* P, unsigned len) {
  uint32_t start = HW_getMicros();
  unsigned rc =  m_sendFile.readBytes( m_buf, len);
  unsigned et =  HW_getMicros() - start;
  if( et > 900) {
    ESP_LOGI(TAG, "Slow file read: rc %lu, len %lu, time %lu", rc, len , time);
  }
  return rc;
}
int HttpServer::clientRead( char* P, unsigned len) {
  uint32_t start = HW_getMicros();
  int rc = m_client->read((uint8_t*) P, len);
  unsigned et =  HW_getMicros() - start;
  if( rc  && et > 900) {
    ESP_LOGI(TAG, "Slow client read: rc %lu, len %lu, time %lu", rc, len , time);
  }
  return rc;
}
void HttpServer::clientWrite( const char* P){
  clientWrite( P, strlen(P));
}
void HttpServer::clientWrite( const char* P, unsigned len){
  uint32_t start = HW_getMicros();
  size_t numWritten = m_client->write( P, len);
  if(numWritten != len) {
    ESP_LOGI(TAG, "Not all written: len %lu, numWritten %lu", len , numWritten);
  }
  unsigned et =  HW_getMicros() - start;
  if(et > 900) {
    ESP_LOGI(TAG, "Slow client write: len %lu, time %lu", len , time);
  }
}
char HttpServer::hexToAscii( const char* h) {
  char rc = charToHex( *h++);
  rc <<= 4;
  rc += charToHex( *h);
  return rc;
}
void HttpServer::sendExec(  uint8_t offset ) {
  uint16_t i;
  for( i = 0; i < (sizeof(m_url) - offset -1) && m_url[ offset + (i*2)] != '\0'; i++) {
    m_url[ offset + i] = hexToAscii( &m_url[ offset + (i *2)]);
  }
  m_url[ offset + i] = '\0';
  const char*p = &m_url[ offset];
  startExec();
  exec( p);
}

void HttpServer::sendFile( const char* type) {
  m_sendFile = LittleFS.open(m_url, "r");
  if( !m_sendFile) {
    send404();
  } else if( m_client != NULL) {
    clientWrite("HTTP/1.0 200 OK\r\nContent-type: ");
    clientWrite(type);
    clientWrite("\r\nCache-Control: max-age=3600\r\n\r\n");
    m_responseCode = 200;
  }
  changeState(STATE_SEND_FILE);
}

void HttpServer::send404(  ) {
  if( m_client != NULL) {
    m_responseCode = 404;
    clientWrite("HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nCache-Control: no-cache\r\n\r\n<!DOCTYPE HTML>\r\n<html><head><title>404 Error</title></head><body><h1>404 Error</h1></body></html>");
  }
  changeState( STATE_DISCONNECTING);
}

void HttpServer::changeState( uint8_t newState) {
  ESP_LOGI(TAG, "Change state from %u to %u", m_state, newState);
  m_state = newState;
}

void HttpServer::slice() {
  uint32_t start = HW_getMicros();
  uint8_t startState = m_state;
  switch( m_state) {
    case STATE_STARTUP:
      // BAM - 20260107 - Need to wait for WiFi to be initialized before creating a server or client
      if(WiFi.getMode() != WIFI_MODE_UNAVAILABLE) {
        m_server = new WiFiServer(m_port);
        m_server->begin();
        changeState( STATE_RESET);
      }
    break;
    case STATE_RESET:
      m_client = new WiFiClient;
      changeState( STATE_IDLE);
      m_responseCode = 0;
      m_urlIndex = 0;
      m_url[ 0] = '\0';
    break;
    case STATE_IDLE:
      *m_client = m_server->accept();
      if( *m_client) {
        m_timer.setInterval( 100);
        ESP_LOGD(TAG, "Connected");
        changeState( STATE_CONNECTING);
      }
    break;
    case STATE_PROCESS_REQUEST:
    {
      m_timer.setInterval( 20000);
      ESP_LOGD(TAG, "Request url: %s", m_url);

      if( strncmp( m_url, "GET ", 4) && strncmp( m_url, "GET ", 4)) {
        send404();
      } else {
        for( uint8_t i = 4; i < sizeof( m_url); i++) {
          char c = m_url[i];
          if( c == ' ' ) {
            m_url[ i - 4] = '\0';
            break;
          } else {
            m_url[ i - 4] = c;
          }
        }
        if( m_url[0] == '/' && m_url[ 1] == '\0') {
          strcpy( m_url, "/index.html");
        }
        if( strlen( m_url) <  4) {
          changeState( STATE_DISCONNECTING);
        } else if( !strncmp( m_url, "/exec/", 6)) {
          sendExec( 6);
          changeState( STATE_PROCESS_EXEC);
        } else if( !strncmp( m_url, "/cmd/", 5)) {
          sendExec( 5);
          changeState( STATE_PROCESS_CMD);
        } else {
          const char* suffix = m_url + strlen( m_url);
          while( suffix > m_url && *suffix != '.') {
            suffix--;
          }
          if(!strcmp( suffix, ".html")) {
            sendFile( "text/html");
          } else if( !strcmp( suffix, ".css")) {
            sendFile( "text/css");
          } else if( !strcmp( suffix, ".js")) {
            sendFile( "text/javascript");
          } else if( !strcmp( suffix, ".ico")) {
            sendFile( "image/x-icon");
          } else if( !strcmp( suffix, ".gif")) {
            sendFile("image/gif");
          } else if( !strcmp( suffix, ".png")) {
            sendFile("image/png");
          } else if( !strcmp( suffix, ".jpg") || !strcmp( suffix, ".jpeg") || !strcmp( suffix, ".jpe")) {
            sendFile("image/jpeg");
          } else {
            send404();
          }
        }
      }
    }
    break;
    case STATE_DISCONNECTING:
      if( m_client != NULL) {
        delete m_client;
        m_client = NULL;
      }
      ESP_LOGD(TAG, "Disconnect");
      m_timer.setInterval( 1);
      changeState( STATE_LOG_DISCONNECT);
    break;
    case STATE_LOG_DISCONNECT:
      {
        uint32_t et = HW_getMicros() - m_requestStart;
        et = (et + 500)/1000;
        ESP_LOGD(TAG, "Request took %lu us to process, ret %lu, url %s", et, m_responseCode, m_url);
      }
      changeState( STATE_DISCONNECT_WAIT);
    break;
    case STATE_DISCONNECT_WAIT:
      changeState( STATE_RESET);
    break;
    case STATE_SEND_FILE:
      if( m_timer.hasIntervalElapsed()) {
        m_sendFile.close();
        changeState( STATE_DISCONNECTING);
      } else {
        //if( m_client != NULL && m_client->availableForWrite() >= (int) sizeof(m_buf)) {
        if( m_client != NULL) {
          size_t br = readFile( m_buf, sizeof(m_buf)); 
          if( br > 0) {
            clientWrite( m_buf, br);
          } else {
            m_sendFile.close();
            changeState( STATE_DISCONNECTING);
          }
        }
      }
    break;

    case STATE_PROCESS_EXEC:
      m_responseCode = 200;
      clientWrite( "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nCache-Control: no-cache\r\n\r\n");
      m_timer.setInterval( 10000);
      changeState( STATE_FINISH_EXEC);
    break;
    case STATE_FINISH_EXEC:
      if( sendExecReply() || m_timer.hasIntervalElapsed()) {
        endExec();
        changeState( STATE_DISCONNECTING);
      }
    break;
    case STATE_PROCESS_CMD:
      m_responseCode = 200;
      clientWrite( "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nAccess-Control-Allow-Origin: *\r\nCache-Control: no-cache\r\n\r\n<!DOCTYPE HTML>\r\n<html><head><title>Cmd</title></head><body><pre>\r\n");
      m_timer.setInterval( 10000);
      changeState( STATE_FINISH_CMD);
    break;
    case STATE_FINISH_CMD:
      if( sendExecReply() || m_timer.hasIntervalElapsed()) {
        clientWrite( "\r\n</pre></body></html>");
        endExec();
        changeState( STATE_DISCONNECTING);
      }
    break;

    case STATE_CONNECTING:
      m_requestStart = HW_getMicros();
      changeState( STATE_CONNECTED);
    break;
    case STATE_CONNECTED:
      if( m_client == NULL) {
        m_responseCode = 1;
        changeState( STATE_DISCONNECTING);
      } else if( m_timer.hasIntervalElapsed( )) {
        m_responseCode = 2;
        changeState( STATE_DISCONNECTING);
      } else if( m_urlIndex >= (sizeof(m_url) - 1) ) {
        m_responseCode = 3;
        changeState( STATE_DISCONNECTING);
      } else {
        int nb = clientRead( &m_url[ m_urlIndex], sizeof(m_url) - m_urlIndex -1);
        if( nb > 0) {
          m_urlIndex += nb;
          m_url[ m_urlIndex ] = '\0';
          bool flag = true;
          for( uint16_t i = 0; flag && i < m_urlIndex; i++) {
            if( m_url[ i] == '\r' || m_url[i] == '\n') {
              m_url[ i] = '\0';
              m_client->clear();
              if( strlen( m_url) < 5 ) {
                changeState( STATE_DISCONNECTING);
              } else {
                changeState( STATE_PROCESS_REQUEST);
              }
              flag = false;
            }
          }
        }
      }
    break;
  }
  unsigned et =  HW_getMicros() - start;
  if( et > 900) {
    ESP_LOGI(TAG, "Slow slice, startState: %lu, state %lu, time %lu", startState, m_state, et);
  }
}
