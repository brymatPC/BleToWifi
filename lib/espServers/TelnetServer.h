#ifndef TelnetServer_h
#define TelnetServer_h

#include <core/CircularQ.h>
#include <core/IntervalTimer.h>

class NetworkServer;
class NetworkClient;

class TelnetServer : public Sliceable {
protected:

  unsigned m_port;
  NetworkServer* m_server;
  NetworkClient* m_client;

  CircularQBase<char>* m_fromTelnetQ;
  CircularQBase<char>* m_toTelnetQ; 

  IntervalTimer m_timer;
  uint8_t m_data0, m_data1, m_state;
  bool m_lastCharWasNull, m_flipFlop, m_lastConnected;

  void changeState( uint8_t newState);

public:
  TelnetServer(void);
  virtual ~TelnetServer();
  virtual const char* sliceName( ) { return "TelnetServer"; }
  void init( unsigned port, CircularQBase<char> *in, CircularQBase<char>* out);
  void slice( void);
};

class CQ1 : public CircularQ<char, 32> {
public:
  CQ1() {}
  virtual ~CQ1() {}
  virtual const char* sliceName( void) { return "CQ1"; } 
  virtual void slice( void) { }
};
class CQ2 : public CircularQ<char, 1024> {
public:
  CQ2() {}
  virtual ~CQ2() {}
  virtual const char* sliceName( void) { return "CQ2"; } 
  virtual void slice( void) { }
};

class TelnetLogServer : public TelnetServer {
private:
  bool m_enabled;
protected:
  CQ1 m_fq;
  CQ2 m_tq;
public:
  TelnetLogServer(void) { m_enabled = false; }
  virtual ~TelnetLogServer() { }
  virtual const char* sliceName( ) { return "TelnetLogServer"; }
  void init( unsigned port);
  void enable( bool enable) { m_enabled = enable; }
  void put( char c ) { if(m_enabled) { m_tq.put( c); } }
  bool spaceAvailable( uint16_t n = 1) { return m_enabled ? m_toTelnetQ->spaceAvailable( n) : false; }
};

#endif