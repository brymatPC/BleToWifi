#ifndef HttpServer_h
#define HttpServer_h

#include <LittleFS.h>

#include <core/Sliceable.h>
#include <core/IntervalTimer.h>

class DebugLog;
class NetworkServer;
class NetworkClient;

class HttpServer : public Sliceable {
protected:
  int m_port;
  uint8_t m_state;
  uint16_t m_responseCode;
  uint32_t m_requestStart;
  char m_url[ 512];
  char m_buf[ 128];
  uint8_t m_urlIndex;

  IntervalTimer m_timer;
  File m_sendFile;
  NetworkServer* m_server;
  NetworkClient* m_client;

  DebugLog* m_log;

  void sendExec( uint8_t offset);
  void sendFile( const char* type);

  virtual void startExec( void) { }
  virtual void endExec( void) { }
  virtual bool sendExecReply( void) { return true; }

  unsigned readFile( char* P, unsigned len);
  void send404( void );
  void changeState( uint8_t newState);
  virtual void exec( const char *p) {}
  int clientRead( char* P, unsigned len);
  void clientWrite( const char* P, unsigned len);
  void clientWrite( const char* P);

public:
  HttpServer(void);
  virtual ~HttpServer( );
  virtual const char* sliceName( ) { return "HttpServer"; }
  virtual void init( unsigned port, DebugLog* log = NULL);
  virtual void slice( void);
  static char hexToAscii( const char* h);
};

#endif