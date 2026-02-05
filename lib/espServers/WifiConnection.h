#ifndef WifiConnection_h
#define WifiConnection_h

#if defined (ESP32)
  #include <Wifi.h>
  #include <Preferences.h>
#else
  #warning "WiFi is not supported on the selected target"
#endif
#include <utility/Sliceable.h>
#include <utility/IntervalTimer.h>
#include <utility/DebugLog.h>
#include <utility/LedDriver.h>

#define MAX_WIFI_ENTRY_LEN 32
#define MAX_WIFI_NETWORKS  4

class WifiConnection : public Sliceable {
private:
  static const char s_PREF_NAMESPACE[];

  char m_hostName[MAX_WIFI_ENTRY_LEN];
  char m_hostPassword[MAX_WIFI_ENTRY_LEN];
  char m_hostIp[MAX_WIFI_ENTRY_LEN];
  char m_hostGateway[MAX_WIFI_ENTRY_LEN];
  char m_hostMask[MAX_WIFI_ENTRY_LEN];

  char m_networkName[MAX_WIFI_NETWORKS][MAX_WIFI_ENTRY_LEN];
  char m_networkPassword[MAX_WIFI_NETWORKS][MAX_WIFI_ENTRY_LEN];

  uint32_t m_networkIp;

protected:
  uint8_t m_currentAp, m_state;
  uint32_t m_connectTimeout;
  int32_t m_maxRssi;
  uint8_t m_maxRssiIndex;
  DebugLog* m_log;
  LedDriver* m_led;
  bool m_enable;
  IntervalTimer m_timer;
  bool m_hostActive;
  void changeState( uint8_t state);
  void hostConfig( void);

public:
  WifiConnection( LedDriver* led, DebugLog* log = NULL, uint32_t connectTimeout = 5000); 
  virtual ~WifiConnection() { }
  virtual const char* sliceName( void) { return "WifiConnection"; }
  virtual void slice( void);
  int getConnectedNetworkIndex( void);
  bool isNetworkConnected( void);

  void setup(Preferences &pref);
  void save(Preferences &pref);

  const char* getHostName( void) { return m_hostName; }
  const char* getHostPassword( void) { return m_hostPassword; }
  const char* getHostIp( void) { return m_hostIp; }
  const char* getHostGateway( void) { return m_hostGateway; }
  const char* getHostMask( void) { return m_hostMask; }
  const char* getNetworkIp( void);
  const char* getNetworkName( uint8_t index);
  const char* getNetworkPassword( uint8_t index);
  void getHostMac( char* buf) { WiFi.softAPmacAddress( (uint8_t*) buf); }
  void getNetworkMac( char* buf) { WiFi.macAddress( (uint8_t*) buf); }
  uint8_t getNumberOfNetworks( void) { return MAX_WIFI_NETWORKS; }

  void setHostName( const char* networkName);
  void setHostPassword( const char* networkPassword );
  void setHostIp( const char* ip );
  void setHostGateway( const char* gateway);
  void setHostMask( const char* mask);
  void setNetworkName( uint8_t index, const char* networkName);
  void setNetworkPassword( uint8_t index, const char* networkPassword);

  bool isHostActive( void) { return m_hostActive; }
  void enable( void) { m_enable = true; }
  void disable( void) { m_enable = false; }
};

#endif
