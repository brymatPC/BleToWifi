#include "YRShellEsp32.h"
#include "LedStripDriver.h"
#include "TelnetServer.h"
#include "TempHumidityParser.h"
#include "WifiConnection.h"
#include "VictronDevice.h"
#include "UploadDataClient.h"
#include <utility/String.h>

#ifdef ESP32
#include <BleConnection.h>
#include <Preferences.h>
#include <time.h>
#endif

#define INITIAL_LOAD_FILE "/start.yr"

static char s_testRoute[] = "/yrshell";

static const FunctionEntry yr8266ShellExtensionFunctions[] = {
    { SE_CC_setPinIn,             "setPinIn" },
    { SE_CC_setPinIn,             "spi" },
    { SE_CC_setPinInPullup,       "setPinInPullup" },
    { SE_CC_setPinInPullup,       "spip" },
    { SE_CC_setPinOut,            "setPinOut" },
    { SE_CC_setPinOut,            "spo" },
    { SE_CC_setDigitalPin,        "setDigitalPin" },
    { SE_CC_setDigitalPin,        "sdp" },
    { SE_CC_setAnalogPin,         "setAnalogPin" },
    { SE_CC_setAnalogPin,         "sap" },
    { SE_CC_getDigitalPin,        "getDigitalPin" },
    { SE_CC_getAnalogPin,         "getAnalogPin" },
    { SE_CC_ledPush,              "ledPush" },
    { SE_CC_ledPop,               "ledPop" },
    { SE_CC_setLedOnOffMs,        "setLedOnOffMs" },
    { SE_CC_setLogMask,           "setLogMask" },   
    { SE_CC_execDone,             "execDone"},
    { SE_CC_hexModeQ,             "hexMode?"},
    { SE_CC_wifiConnected,        "wifiConnected"},
    { SE_CC_setTelnetLogEnable,   "setTelnetLogEnable"},

    { SE_CC_getHostName,          "getHostName" },
    { SE_CC_getHostPassword,      "getHostPassword" },
    { SE_CC_getHostIp,            "getHostIp" },
    { SE_CC_getHostGateway,       "getHostGateway" },
    { SE_CC_getHostMask,          "getHostMask" },
    { SE_CC_getHostMac,           "getHostMac" },
    { SE_CC_isHostActive,         "isHostActive" },

    { SE_CC_getNumberOfNetworks,  "getNumberOfNetworks" },
    { SE_CC_getConnectedNetwork,  "getConnectedNetwork" },
    { SE_CC_getNetworkIp,         "getNetworkIp" },
    { SE_CC_getNetworkMac,        "getNetworkMac" },
    { SE_CC_getNetworkName,       "getNetworkName" },
    { SE_CC_getNetworkPassword,   "getNetworkPassword" },

    { SE_CC_setHostName,          "setHostName" },
    { SE_CC_setHostPassword,      "setHostPassword" },
    { SE_CC_setHostIp,            "setHostIp" },
    { SE_CC_setHostGateway,       "setHostGateway" },
    { SE_CC_setHostMask,          "setHostMask" },
    { SE_CC_setNetworkName,       "setNetworkName" },
    { SE_CC_setNetworkPassword,   "setNetworkPassword" },

    { SE_CC_saveNetworkParameters,"saveNetworkParameters" },

    { SE_CC_loadFile,              "loadFile" },

    { SE_CC_dbgM,                  "logM" },
    { SE_CC_dbgDM,                 "logDM" }, 
    { SE_CC_dbgDDM,                "logDDM" }, 
    { SE_CC_dbgXM,                 "logXM" }, 
    { SE_CC_dbgXXM,                "logXXM" },  

    { SE_CC_hardReset,             "hardReset" },  

    { SE_CC_dotUb,                ".ub" },
    { SE_CC_strToInt,             "strToInt"},

    { SE_CC_checkPreferences,     "checkPref"},
    { SE_CC_storePreferences,     "storePref"},

    { SE_CC_bleScan,                 "bscan"},
    { SE_CC_setBleLogState,          "sbls"},
    { SE_CC_setBleScanInterval,      "setBleScanInterval"},
    { SE_CC_setBleScanWindow,        "setBleScanWindow"},
    { SE_CC_setBleDuration,          "setBleDuration"},
    { SE_CC_setBleScanActively,      "setBleScanActively"},
    { SE_CC_setBleScanStartInterval, "setBleScanStartInterval"},
    { SE_CC_setBleAddr,              "setBleAddr"},
    { SE_CC_setBleParser,            "setBleParser"},
    { SE_CC_setBleEnable,            "setBleEnable"},
    { SE_CC_logBleParsers,           "logBleParsers"},

    { SE_CC_setVicKey,               "svk"},
    { SE_CC_setTempHumidityLogging,  "setTHLogging"},

    { SE_CC_setUploadIp,            "setUploadIp"},
    { SE_CC_setUploadPort,          "setUploadPort"},

    { SE_CC_flashSize,            "flashSize"},
    { SE_CC_curTime,              "curTime"},
		{ SE_CC_setTime,              "setTime"},

    { SE_CC_upload,               "upload"},
    { SE_CC_setLedStrip,          "setLedStrip"},

    { 0, NULL}
};


static FunctionDictionary dictionaryExtensionFunction( yr8266ShellExtensionFunctions, YRSHELL_DICTIONARY_EXTENSION_FUNCTION );

CompiledDictionary compiledExtensionDictionary( NULL, 0xFFFF , 0x0000 , YRSHELL_DICTIONARY_EXTENSION_COMPILED);

static char s_uploadData[] = "{\"data\":32}";

YRShellEsp32::YRShellEsp32() {
  m_telnetLogServer = NULL;
  m_fileOpen = false;
  m_initialFileLoaded = false;
  m_initialized = false;
  m_auxBufIndex = 0;
}

YRShellEsp32::~YRShellEsp32() {
}

void YRShellEsp32::init( DebugLog* log) {
  YRShellBase::init();
  m_dictionaryList[ YRSHELL_DICTIONARY_EXTENSION_COMPILED_INDEX] = &compiledExtensionDictionary;
  m_dictionaryList[ YRSHELL_DICTIONARY_EXTENSION_FUNCTION_INDEX] = &dictionaryExtensionFunction;
  m_log = log;
  m_exec = false;
  m_initialized = true;
}

void YRShellEsp32::startExec( void) {
  m_lastPromptEnable = getPromptEnable();
  m_lastCommandEcho = getCommandEcho();
  setPromptEnable( false);
  setCommandEcho( false);
}
void YRShellEsp32::endExec( void) {
  setPromptEnable( m_lastPromptEnable);
  setCommandEcho( m_lastCommandEcho);
  requestUseMainQueues();
}
void YRShellEsp32::execString( const char* p) {
  if( m_exec) {
      m_log->print( __FILE__, __LINE__, 1, "YRShellEsp32_execString_failed: ");
  } else {
    requestUseAuxQueues();
    for( ; *p != '\0'; p++ ) {
      m_AuxInq->put( *p);
    }
    for( const char* p = " cr execDone\r"; *p != '\0'; p++ ) {
      m_AuxInq->put( *p);
    }
    m_exec = true;
    m_execTimer.setInterval( 5000);
  }
}

void YRShellEsp32::loadFile( const char* fname, bool exec) {
  if( m_fileOpen) {
    if( m_log != NULL) {
      m_log->print( __FILE__, __LINE__, 1, "YRShellEsp32_loadFile_Failed: ");
    }
  } else {
    if( fname == NULL || fname[0] == '\0') {
      m_log->print( __FILE__, __LINE__, 1, "YRShellEsp32_loadFile_no_valid_file: ");
    } else {
      m_file = LittleFS.open(fname, "r");
      if( !m_file) {
        if( m_log != NULL) {
          m_log->print( __FILE__, __LINE__, 1, fname, "YRShellEsp32_loadFile_Failed: fname");
        }
      } else {
        if( exec) {
          requestUseAuxQueues();
        }
        m_fileOpen = true;
        if( m_log != NULL) {
          m_log->print( __FILE__, __LINE__, 1, fname, "YRShellEsp32_loadFile_loading: fname");
        }
      }
    }
  }
}

void YRShellEsp32::slice() {
  YRShellBase::slice();
  if( m_fileOpen && m_auxInq.spaceAvailable(10)) {
    int c = m_file.read();
    if( c != -1) {
      m_auxInq.put( c);   
    } else {
      m_file.close();
      m_fileOpen = false;
      m_log->print( __FILE__, __LINE__, 1, "YRShellEsp32_slice_closing_file");
    } 
  }

  if( m_exec && m_execTimer.hasIntervalElapsed()) {
    m_exec = false;
  }
  
  if( m_useAuxQueues && !m_exec ) {
    while( m_AuxOutq->valueAvailable()) {
      char c = m_AuxOutq->get();
      if( c != '\r' && c != '\n' ) {
        m_auxBuf[ m_auxBufIndex++] = c;    
      }
      if( c == '\r' || c == '\n' ||  m_auxBufIndex > (sizeof(m_auxBuf) - 2 ) ) {
        m_auxBuf[ m_auxBufIndex] = '\0';
        bool flag = true;
        for( const char* p = m_auxBuf; flag && *p != '\0'; p++) {
          if( *p != ' ' && *p != '\r' && *p != '\n' && *p != '\t') {
            flag = false;
          }
        }
        if( !flag && m_log != NULL) {
          m_log->print( __FILE__, __LINE__, 8, m_auxBuf, "YRShellEsp32_slice: auxBuf");
        }
        m_auxBufIndex = 0;
      }
    }
  } else if( m_auxBufIndex > 0) {
    if( m_log != NULL) {
      m_log->print( __FILE__, __LINE__, 8, m_auxBuf, "YRShellEsp32_slice: auxBuf");
    }
    m_auxBufIndex = 0;
  }

  if( !m_initialFileLoaded && m_initialized && isIdle() ) {
      m_initialFileLoaded = true;
      loadFile( INITIAL_LOAD_FILE);
  }
} 



void YRShellEsp32::executeFunction( uint16_t n) {
  uint32_t t1, t2;
  if( n <= SE_CC_first || n >= SE_CC_last) {
      YRShellBase::executeFunction(n);
  } else {
      switch( n) {
          case SE_CC_setPinIn:
              pinMode(popParameterStack(), INPUT);
              break;
          case SE_CC_setPinInPullup:
              pinMode(popParameterStack(), INPUT_PULLUP);
              break;
          case SE_CC_setPinOut:
              pinMode(popParameterStack(), OUTPUT);
              break;
          case SE_CC_setDigitalPin:
              t1 = popParameterStack();
              t2 = popParameterStack();
              digitalWrite( t1, t2);
              break;
          case SE_CC_setAnalogPin:
              t1 = popParameterStack();
              t2 = popParameterStack();
              analogWrite( t1, t2);
              break;
          case SE_CC_getDigitalPin:
              pushParameterStack( digitalRead(popParameterStack()));
              break;
          case SE_CC_getAnalogPin:
#ifdef ESP8266
              pushParameterStack( analogRead(A0));
#else
              pushParameterStack( 0);
#endif
              break;
          case SE_CC_ledPush:
              if( m_led) {
                m_led->push();
              }
          break;
          case SE_CC_ledPop:
              if( m_led) {
                m_led->pop();
              }
          break;
          case SE_CC_setLedOnOffMs:
              t1 = popParameterStack();
              t2 = popParameterStack();
              if( m_led) {
                m_led->setLedOnOffMs( t2, t1);
              }
              break;
          case SE_CC_execDone:
              m_exec = false;
              break;
          case SE_CC_setLogMask:
              t1 = popParameterStack();
              if( m_log != NULL) {
                m_log->setMask( t1);
              }
              break;
          case SE_CC_hexModeQ:
              pushParameterStack( m_hexMode);
              break;
          case SE_CC_wifiConnected:
              pushParameterStack(  WiFi.status() == WL_CONNECTED);
              break;
          case SE_CC_setTelnetLogEnable:
              t1 = popParameterStack();
              if( m_telnetLogServer) {
                m_telnetLogServer->enable(t1);
              }
              break;

          case SE_CC_getHostName:
              m_textBuffer[ 0] = '\0';
              if( m_wifiConnection) {
                strcpy( m_textBuffer, m_wifiConnection->getHostName());
              }
              pushParameterStack( 0);
            break;
          case SE_CC_getHostPassword:
              m_textBuffer[ 0] = '\0';
              if( m_wifiConnection) {
                strcpy( m_textBuffer, m_wifiConnection->getHostPassword());
              }
              pushParameterStack( 0);
            break;
          case SE_CC_getHostIp:
              m_textBuffer[ 0] = '\0';
              if( m_wifiConnection) {
                strcpy( m_textBuffer, m_wifiConnection->getHostIp());
              }
              pushParameterStack( 0);
              break;
          case SE_CC_getHostGateway:
              m_textBuffer[ 0] = '\0';
              if( m_wifiConnection) {
                strcpy( m_textBuffer, m_wifiConnection->getHostGateway());
              }
              pushParameterStack( 0);
              break;
          case SE_CC_getHostMask:
              m_textBuffer[ 0] = '\0';
              if( m_wifiConnection) {
                strcpy( m_textBuffer, m_wifiConnection->getHostMask());
              }
              pushParameterStack( 0);
              break;
          case SE_CC_getHostMac:
              m_textBuffer[ 0] = '\0';
              if( m_wifiConnection) {
                m_wifiConnection->getHostMac(m_textBuffer);
              }
              pushParameterStack( 0);
              break;

          case SE_CC_isHostActive:
              if( m_wifiConnection) {
                pushParameterStack( m_wifiConnection->isHostActive());
              } else {
                pushParameterStack( 0);
              }
              break;
              break;

          case SE_CC_getNumberOfNetworks:
              if( m_wifiConnection) {
                pushParameterStack( m_wifiConnection->getNumberOfNetworks());
              } else {
                pushParameterStack( 0);
              }
              break;
          case SE_CC_getConnectedNetwork:
              if( m_wifiConnection) {
                pushParameterStack( m_wifiConnection->getConnectedNetworkIndex());
              } else {
                pushParameterStack( -1);
              }
              break;
          case SE_CC_getNetworkIp:
              m_textBuffer[ 0] = '\0';
              if( m_wifiConnection) {
                strcpy( m_textBuffer, m_wifiConnection->getNetworkIp( ));
              }
              pushParameterStack( 0);
              break;
          case SE_CC_getNetworkMac:
              m_textBuffer[ 0] = '\0';
              if( m_wifiConnection) {
                m_wifiConnection->getNetworkMac( m_textBuffer );
              }
              pushParameterStack( 0);
              break;
          case SE_CC_getNetworkName:
              t1 = popParameterStack();
              m_textBuffer[ 0] = '\0';
              if( m_wifiConnection) {
                strcpy( m_textBuffer, m_wifiConnection->getNetworkName( t1));
              }
              pushParameterStack( 0);
              break;
          case SE_CC_getNetworkPassword:
              t1 = popParameterStack();
              m_textBuffer[ 0] = '\0';
              if( m_wifiConnection) {
                strcpy( m_textBuffer, m_wifiConnection->getNetworkPassword( t1));
              }
              pushParameterStack( 0);
              break;
          case SE_CC_setHostName:
              t1 = popParameterStack();
              if( m_wifiConnection) {
                  m_wifiConnection->setHostName( getAddressFromToken( t1));
              }
              break;
          case SE_CC_setHostPassword:
              t1 = popParameterStack();
              if( m_wifiConnection) {
                  m_wifiConnection->setHostPassword( getAddressFromToken( t1));
              }
              break;
          case SE_CC_setHostIp:
              t1 = popParameterStack();
              if( m_wifiConnection) {
                  m_wifiConnection->setHostIp( getAddressFromToken( t1));
              }
              break;
          case SE_CC_setHostGateway:
              t1 = popParameterStack();
              if( m_wifiConnection) {
                  m_wifiConnection->setHostGateway( getAddressFromToken( t1));
              }
              break;
          case SE_CC_setHostMask:
              t1 = popParameterStack();
              if( m_wifiConnection) {
                  m_wifiConnection->setHostMask( getAddressFromToken( t1));
              }
              break;
          case SE_CC_setNetworkName:
              t1 = popParameterStack();
              t2 = popParameterStack();
              if( m_wifiConnection) {
                  m_wifiConnection->setNetworkName( t1, getAddressFromToken( t2));
              }
              break;
          case SE_CC_setNetworkPassword: 
              t1 = popParameterStack();
              t2 = popParameterStack();
              if( m_wifiConnection) {
                  m_wifiConnection->setNetworkPassword( t1, getAddressFromToken( t2));
              }
              break;
          case SE_CC_saveNetworkParameters: 
              if( m_wifiConnection && m_pref) {
                  m_wifiConnection->save(*m_pref);
              }
              break;

          case SE_CC_loadFile:
              loadFile( getAddressFromToken(popParameterStack()), false );
              break;
          case SE_CC_dbgM:
              if( m_log) {
                m_log->print( __FILE__, __LINE__, 1, getAddressFromToken(popParameterStack()) );
              }
              break;
          case SE_CC_dbgDM:
              t1 = popParameterStack();
              if( m_log) {
                m_log->print( __FILE__, __LINE__, 1, t1, getAddressFromToken(popParameterStack()) );
              }
              break;
          case SE_CC_dbgDDM:
              t1 = popParameterStack();
              t2 = popParameterStack();
              if( m_log) {
                m_log->print( __FILE__, __LINE__, 1, t2, t1,  getAddressFromToken(popParameterStack()) );
              }
              break;
          case SE_CC_dbgXM:
              t1 = popParameterStack();
              if( m_log) {
                m_log->printX( __FILE__, __LINE__, 1, t1, getAddressFromToken(popParameterStack()) );
              }
              break;
          case SE_CC_dbgXXM:
              t1 = popParameterStack();
              t2 = popParameterStack();
              if( m_log) {
                m_log->printX( __FILE__, __LINE__, 1, t2, t1,  getAddressFromToken(popParameterStack()) );
              }
              break;

          case SE_CC_hardReset:
              LittleFS.remove( "/NetworkParameters");
              ESP.restart();
              break;

          case SE_CC_dotUb:
              outUint8( popParameterStack());
              break;

          case SE_CC_strToInt:
              if( m_textBuffer[0] == '0' && m_textBuffer[ 1] == 'x') {
                stringToUnsignedX( m_textBuffer, &t1 );
              } else {
                stringToUnsigned( m_textBuffer, &t1 );
              }
              pushParameterStack( t1);
              break;
          case SE_CC_checkPreferences:
              if(m_pref) {
                pushParameterStack(m_pref->freeEntries());
              } else {
                pushParameterStack(0);
              }
              break;
          case SE_CC_storePreferences:
              if(m_pref) {
                if(m_wifiConnection) {
                  m_wifiConnection->save(*m_pref);
                }
                if( m_bleConnection ) {
                  m_bleConnection->save(*m_pref);
                }
                if(m_victronDevice) {
                  m_victronDevice->save(*m_pref);
                }
                if(m_uploadClient) {
                  m_uploadClient->save(*m_pref);
                }
              }
              break;
          case SE_CC_bleScan:
              if( m_bleConnection ) {
                m_bleConnection->requestScan();
              }
            break;
          case SE_CC_setBleLogState:
              t1 = popParameterStack();
              if( m_bleConnection ) {
                m_bleConnection->setLogState((bleLogState)t1);
              }
              break;
          case SE_CC_setBleScanInterval:
              t1 = popParameterStack();
              if( m_bleConnection ) {
                m_bleConnection->setScanInterval((uint16_t)t1);
              }
              break;
          case SE_CC_setBleScanWindow:
              t1 = popParameterStack();
              if( m_bleConnection ) {
                m_bleConnection->setScanWindow((uint16_t)t1);
              }
              break;
          case SE_CC_setBleDuration:
              t1 = popParameterStack();
              if( m_bleConnection ) {
                m_bleConnection->setScanDuration(t1);
              }
              break;
          case SE_CC_setBleScanActively:
              t1 = popParameterStack();
              if( m_bleConnection ) {
                m_bleConnection->setScanActively(t1);
              }
              break;
          case SE_CC_setBleScanStartInterval:
              t1 = popParameterStack();
              if( m_bleConnection ) {
                m_bleConnection->setScanStartInterval(t1);
              }
              break;
          case SE_CC_setBleAddr:
              t1 = popParameterStack();
              t2 = popParameterStack();
              if( m_bleConnection) {
                  m_bleConnection->setBleAddress(t2, getAddressFromToken( t1));
              }
              break;
          case SE_CC_setBleParser:
              t1 = popParameterStack();
              t2 = popParameterStack();
              if( m_bleConnection) {
                  if(t1 == static_cast<uint32_t>(BleParserTypes::tempHumidity)) {
                    m_bleConnection->setBleParser(t2, BleParserTypes::tempHumidity);
                  } else if(t1 == static_cast<uint32_t>(BleParserTypes::victron)) {
                    m_bleConnection->setBleParser(t2, BleParserTypes::victron);
                  } else {
                    m_bleConnection->setBleParser(t2, BleParserTypes::none);
                  }
              }
              break;
          case SE_CC_setBleEnable:
              t1 = popParameterStack();
              t2 = popParameterStack();
              if( m_bleConnection) {
                  m_bleConnection->setBleEnable(t2, t1);
              }
              break;
          case SE_CC_logBleParsers:
              if(m_bleConnection) {
                m_bleConnection->logParsers();
              }
              break;
          case SE_CC_setVicKey:
              t1 = popParameterStack();
              if( m_victronDevice) {
                  m_victronDevice->setKey( getAddressFromToken( t1));
              }
            break;
          case SE_CC_setTempHumidityLogging:
              t1 = popParameterStack();
              if( m_tempHumParser) {
                m_tempHumParser->enableAdditionalLogging(t1);
              }
              break;
          case SE_CC_setUploadIp:
              t1 = popParameterStack();
              if( m_uploadClient) {
                  m_uploadClient->setHostIp( getAddressFromToken( t1));
              }
              break;
          case SE_CC_setUploadPort:
              t1 = popParameterStack();
              if( m_uploadClient) {
                  m_uploadClient->setHostPort(t1);
              }
              break;
          case SE_CC_flashSize:
              t1 = LittleFS.totalBytes();
              t2 = LittleFS.usedBytes();
              pushParameterStack( t1);
              pushParameterStack( t2);
            break;
          case SE_CC_curTime:
              logTime();
            break;
          case SE_CC_setTime:
            {
              struct tm t = {0, 0, 0, 0, 0, 0, 0, 0, 0};
              struct timeval tv;
              t.tm_sec = popParameterStack( );
              t.tm_min = popParameterStack( );
              t.tm_hour = popParameterStack( );
              t.tm_mday = popParameterStack( );
              t.tm_mon = popParameterStack( ) - 1;
              t.tm_year = popParameterStack( ) - 1900;
              tv.tv_sec = mktime(&t);
              tv.tv_usec = 0;
              settimeofday(&tv, NULL);
            }
            break;
          case SE_CC_upload:
            if(m_uploadClient) {
              m_uploadClient->sendFile(s_testRoute, s_uploadData, strlen(s_uploadData));
            }
            break;
          case SE_CC_setLedStrip:
              t1 = popParameterStack();
              if( m_ledStrip) {
                  m_ledStrip->setLed(t1);
              }
            break;
          default:
              shellERROR(__FILE__, __LINE__);
              break;
      }
  }
}

void YRShellEsp32::outUInt8( int8_t v) {
    char buf[ 5];
    unsignedToString(v, 3, buf);
    outString( buf);
}

void YRShellEsp32::logTime() {
    char s[51];
    struct tm timeinfo;
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    //time_t tt = mktime (&timeinfo);

    strftime(s, 50, "%FT%H:%M:%SZ", &timeinfo);
    m_log->print( __FILE__, __LINE__, 1, s, "logTime: rtc");
}