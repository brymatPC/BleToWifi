#ifndef YRSHELL_ESP32_H_
#define YRSHELL_ESP32_H_

#include <YRShell.h>
#include <utility/DebugLog.h>
#include <LittleFS.h>

#include "YRShellExec.h"

class Preferences;
class AppManager;
class LedDriver;
class LedStripDriver;
class WifiConnection;
class TelnetLogServer;
class UploadDataClient;
class BleConnection;
class VictronDevice;
class TempHumidityParser;

typedef enum {
    SE_CC_first = YRSHELL_DICTIONARY_EXTENSION_FUNCTION,
    SE_CC_setPinIn,
    SE_CC_setPinInPullup,
    SE_CC_setPinOut,
    SE_CC_setDigitalPin,
    SE_CC_setAnalogPin,
    SE_CC_getDigitalPin,
    SE_CC_getAnalogPin,
    SE_CC_ledPush,
    SE_CC_ledPop,
    SE_CC_setLedOnOffMs,
    SE_CC_setLogMask,
    SE_CC_execDone,
    SE_CC_hexModeQ,
    SE_CC_wifiConnected,
    SE_CC_setTelnetLogEnable,
    SE_CC_deepSleep,

    SE_CC_setRunTimeMs,
    SE_CC_setSleepTimeMs,
    SE_CC_setSleepEnable,

    SE_CC_getHostName,
    SE_CC_getHostPassword,
    SE_CC_getHostIp,
    SE_CC_getHostGateway,
    SE_CC_getHostMask,
    SE_CC_getHostMac,
    SE_CC_isHostActive,

    SE_CC_getNumberOfNetworks,
    SE_CC_getConnectedNetwork,
    SE_CC_getNetworkIp,
    SE_CC_getNetworkMac,

    SE_CC_getNetworkName,
    SE_CC_getNetworkPassword,

    SE_CC_setHostName,
    SE_CC_setHostPassword,
    SE_CC_setHostIp,
    SE_CC_setHostGateway,
    SE_CC_setHostMask,
    SE_CC_setNetworkName,
    SE_CC_setNetworkPassword,

    SE_CC_saveNetworkParameters,

    SE_CC_loadFile,

    SE_CC_dbgM,
    SE_CC_dbgDM,
    SE_CC_dbgDDM,
    SE_CC_dbgXM,
    SE_CC_dbgXXM,

    SE_CC_hardReset,

    SE_CC_dotUb,
    SE_CC_strToInt,

    SE_CC_checkPreferences,
    SE_CC_storePreferences,

    SE_CC_bleScan,
    SE_CC_setBleLogState,
    SE_CC_setBleScanInterval,
    SE_CC_setBleScanWindow,
    SE_CC_setBleDuration,
    SE_CC_setBleScanActively,
    SE_CC_setBleScanStartInterval,
    SE_CC_setBleScanBoot,
    SE_CC_setBleAddr,
    SE_CC_setBleParser,
    SE_CC_setBleEnable,
    SE_CC_logBleParsers,
    SE_CC_setVicKey,
    SE_CC_setTempHumidityLogging,

    SE_CC_setUploadIp,
    SE_CC_setUploadPort,

    SE_CC_flashSize,
    SE_CC_curTime,
    SE_CC_setTime,

    SE_CC_upload,
    SE_CC_setLedStrip,
    
    SE_CC_last
} SE_CC_functions;

class YRShellEsp32 : public YRShellExec, public virtual YRShellBase<2048, 128, 128, 16, 16, 16, 8, 256, 512, 256, 512, 128> {
protected:
  bool m_exec, m_initialized;
  char m_auxBuf[ 128];
  uint8_t m_auxBufIndex;

  Preferences* m_pref;
  TelnetLogServer* m_telnetLogServer;
  AppManager* m_appMgr;
  LedDriver* m_led;
  LedStripDriver* m_ledStrip;
  WifiConnection* m_wifiConnection;
  BleConnection* m_bleConnection;
  VictronDevice* m_victronDevice;
  TempHumidityParser *m_tempHumParser;
  UploadDataClient* m_uploadClient;
  IntervalTimer m_execTimer;
  bool m_fileOpen, m_initialFileLoaded, m_lastPromptEnable, m_lastCommandEcho;
  File m_file;

  virtual void executeFunction( uint16_t n);
  virtual const char* shellClass( void) { return "YRShellEsp32"; }
  virtual const char* mainFileName( ) { return "main.cpp"; }
  void outUInt8( int8_t v);

  void logTime();

public:
  YRShellEsp32( );
  virtual ~YRShellEsp32( );
  void init( DebugLog* log);

  // Provide object instances to drive testing, can be nullptr
  void setPreferences(Preferences *pref) { m_pref = pref; }
  void setAppMgr(AppManager *appMgr) { m_appMgr = appMgr; }
  void setLedDriver(LedDriver *led) { m_led = led; }
  void setLedStrip(LedStripDriver *strip) { m_ledStrip = strip; }
  void setWifiConnection(WifiConnection* wifiConnection) { m_wifiConnection = wifiConnection; }
  void setBleConnection(BleConnection* bleConnection) { m_bleConnection = bleConnection; }
  void setTelnetLogServer(TelnetLogServer* telnetLogServer) { m_telnetLogServer = telnetLogServer; }
  void setVictronDevice(VictronDevice *device) { m_victronDevice = device; }
  void setTempHumParser(TempHumidityParser *parser) { m_tempHumParser = parser; }
  void setUploadClient(UploadDataClient *client) { m_uploadClient = client; }

  virtual void slice( void);
  void loadFile( const char* fname, bool exec = true);

  inline bool isAuxQueueInUse( void) { return m_useAuxQueues; }
  CircularQBase<char>& getAuxOutq(void) { return *m_AuxOutq; };
  void startExec( void);
  void endExec( void);
  void execString( const char* p);
  bool isExec( void) { return m_exec; }
};

#endif