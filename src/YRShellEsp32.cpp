#include "YRShellEsp32.h"
#include "AppManager.h"
#include "LedStripDriver.h"
#include "TelnetServer.h"
#include "TempHumidityParser.h"
#include "WifiConnection.h"
#include "VictronDevice.h"
#include "UploadDataClient.h"

#ifdef ESP32
#include <BleConnection.h>
#include <Preferences.h>
#include <time.h>
#include <esp_chip_info.h>
#include <esp_idf_version.h>
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
    { SE_CC_deepSleep,            "deepSleep"},
    { SE_CC_eLogLevel,            "eLogLevel"},

    { SE_CC_setRunTimeMs,         "setRunTime"},
    { SE_CC_setSleepTimeMs,       "setSleepTime"},
    { SE_CC_setSleepEnable,       "setSleepEnable"},

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
    { SE_CC_setBleScanBoot,          "setBleScanBoot"},
    { SE_CC_setBleAddr,              "setBleAddr"},
    { SE_CC_setBleParser,            "setBleParser"},
    { SE_CC_setBleEnable,            "setBleEnable"},
    { SE_CC_logBleParsers,           "logBleParsers"},

    { SE_CC_setVicKey,               "svk"},
    { SE_CC_setTempHumidityLogging,  "setTHLogging"},

    { SE_CC_setUploadIp,            "setUploadIp"},
    { SE_CC_setUploadPort,          "setUploadPort"},

    { SE_CC_flashSize,            "flashSize"},
    { SE_CC_chipInfo,             "chipInfo"},
    { SE_CC_sdkVersion,           "sdkVersion"},
    { SE_CC_numTasks,             "numTasks"},
    { SE_CC_cpuPerf,              "cpuPerf"},
    { SE_CC_curTime,              "curTime"},
		{ SE_CC_setTime,              "setTime"},

    { SE_CC_upload,               "upload"},
    { SE_CC_setLedStrip,          "setLedStrip"},

    { 0, NULL}
};


static FunctionDictionary dictionaryExtensionFunction( yr8266ShellExtensionFunctions, YRSHELL_DICTIONARY_EXTENSION_FUNCTION );

CompiledDictionary compiledExtensionDictionary( NULL, 0xFFFF , 0x0000 , YRSHELL_DICTIONARY_EXTENSION_COMPILED);

static char s_uploadData[] = "{\"data\":32}";

/**
 * @brief   Function to print the CPU usage of tasks over a given duration.
 *
 * This function will measure and print the CPU usage of tasks over a specified
 * number of ticks (i.e. real time stats). This is implemented by simply calling
 * uxTaskGetSystemState() twice separated by a delay, then calculating the
 * differences of task run times before and after the delay.
 *
 * @note    If any tasks are added or removed during the delay, the stats of
 *          those tasks will not be printed.
 * @note    This function should be called from a high priority task to minimize
 *          inaccuracies with delays.
 * @note    When running in dual core mode, each core will correspond to 50% of
 *          the run time.
 *
 * @param   xTicksToWait    Period of stats measurement
 *
 * @return
 *  - ESP_OK                Success
 *  - ESP_ERR_NO_MEM        Insufficient memory to allocated internal arrays
 *  - ESP_ERR_INVALID_SIZE  Insufficient array size for uxTaskGetSystemState. Trying increasing ARRAY_SIZE_OFFSET
 *  - ESP_ERR_INVALID_STATE Delay duration too short
 */
#define ARRAY_SIZE_OFFSET   5   //Increase this if print_real_time_stats returns ESP_ERR_INVALID_SIZE
#define MAX_TASKS_TO_TRACK  20
static char s_espBuf[128];
static esp_err_t print_real_time_stats(TickType_t xTicksToWait, DebugLog *dbg)
{
    TaskStatus_t start_array[MAX_TASKS_TO_TRACK];
    TaskStatus_t end_array[MAX_TASKS_TO_TRACK];
    UBaseType_t start_array_size, end_array_size;
    configRUN_TIME_COUNTER_TYPE start_run_time, end_run_time;
    esp_err_t ret;

    if(uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET >= MAX_TASKS_TO_TRACK) {
      dbg->print(__FILE__, __LINE__, 1, uxTaskGetNumberOfTasks(), ARRAY_SIZE_OFFSET, "cpu stats: numTasks, array offset");
      return ESP_ERR_NO_MEM;
    }

    //Allocate array to store current task states
    start_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    // start_array = malloc(sizeof(TaskStatus_t) * start_array_size);
    // if (start_array == NULL) {
    //     ret = ESP_ERR_NO_MEM;
    //     goto exit;
    // }
    //Get current task states
    start_array_size = uxTaskGetSystemState(start_array, start_array_size, &start_run_time);
    if (start_array_size == 0) {
        dbg->print(__FILE__, __LINE__, 1, "cpu stats: start array size is 0");
        return ESP_ERR_INVALID_SIZE;
    }

    vTaskDelay(xTicksToWait);

    //Allocate array to store tasks states post delay
    end_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    // end_array = malloc(sizeof(TaskStatus_t) * end_array_size);
    // if (end_array == NULL) {
    //     ret = ESP_ERR_NO_MEM;
    //     goto exit;
    // }
    //Get post delay task states
    end_array_size = uxTaskGetSystemState(end_array, end_array_size, &end_run_time);
    if (end_array_size == 0) {
      dbg->print(__FILE__, __LINE__, 1, "cpu stats: end array size is 0");
        return ESP_ERR_INVALID_SIZE;
    }

    //Calculate total_elapsed_time in units of run time stats clock period.
    uint32_t total_elapsed_time = (end_run_time - start_run_time);
    if (total_elapsed_time == 0) {
        return ESP_ERR_INVALID_STATE;
    }

    dbg->print(__FILE__, __LINE__, 1, "| Task | Run Time | Core | Percentage");
    //Match each task in start_array to those in the end_array
    for (int i = 0; i < start_array_size; i++) {
        int k = -1;
        for (int j = 0; j < end_array_size; j++) {
            if (start_array[i].xHandle == end_array[j].xHandle) {
                k = j;
                //Mark that task have been matched by overwriting their handles
                start_array[i].xHandle = NULL;
                end_array[j].xHandle = NULL;
                break;
            }
        }
        //Check if matching task found
        if (k >= 0) {
            uint32_t task_elapsed_time = end_array[k].ulRunTimeCounter - start_array[i].ulRunTimeCounter;
            uint32_t percentage_time = (task_elapsed_time * 100UL) / (total_elapsed_time * CONFIG_FREERTOS_NUMBER_OF_CORES);
            snprintf(s_espBuf, 128, "| %s | %" PRIu32" | %d | %" PRIu32"%%", start_array[i].pcTaskName, task_elapsed_time, start_array[i].xCoreID, percentage_time);
            dbg->print(__FILE__, __LINE__, 1, s_espBuf);
        }
    }

    //Print unmatched tasks
    for (int i = 0; i < start_array_size; i++) {
        if (start_array[i].xHandle != NULL) {
            snprintf(s_espBuf, 128, "| %s | Deleted", start_array[i].pcTaskName);
            dbg->print(__FILE__, __LINE__, 1, s_espBuf);
        }
    }
    for (int i = 0; i < end_array_size; i++) {
        if (end_array[i].xHandle != NULL) {
            snprintf(s_espBuf, 128, "| %s | Created", end_array[i].pcTaskName);
            dbg->print(__FILE__, __LINE__, 1, s_espBuf);
        }
    }
    return ESP_OK;
}

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
          case SE_CC_deepSleep:
              t1 = popParameterStack();
              if(m_log) {
                m_log->print(__FILE__, __LINE__, 1, t1, "Entering deep sleep, timeMs");
              }
              if(m_bleConnection) {
                m_bleConnection->off();
              }
              if(m_wifiConnection) {
                m_wifiConnection->off();
              }
              esp_deep_sleep(1000LL * t1);
              break;
          case SE_CC_eLogLevel:
              t1 = popParameterStack();
              if(t1 >=ESP_LOG_NONE && t1<=ESP_LOG_MAX) {
                esp_log_level_set("*", static_cast<esp_log_level_t>(t1));
              }
          break;
          case SE_CC_setRunTimeMs:
              t1 = popParameterStack();
              if(m_appMgr) {
                m_appMgr->setRunTimeMs(t1);
              }
          break;
          case SE_CC_setSleepTimeMs:
              t1 = popParameterStack();
              if(m_appMgr) {
                m_appMgr->setSleepTimeMs(t1);
              }
          break;
          case SE_CC_setSleepEnable:
              t1 = popParameterStack();
              if(m_appMgr) {
                m_appMgr->setSleepEnabled(t1);
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
                if(m_appMgr) {
                  m_appMgr->save(*m_pref);
                }
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
          case SE_CC_setBleScanBoot:
              t1 = popParameterStack();
              if( m_bleConnection ) {
                m_bleConnection->setScanStartBoot(t1);
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
          case SE_CC_chipInfo:
            {
              esp_chip_info_t chipInfo;
              esp_chip_info(&chipInfo);
              m_log->print( __FILE__, __LINE__, 1, chipInfo.model, chipInfo.revision, chipInfo.cores, "Chip: model, revision, cores");
            }
            break;
          case SE_CC_sdkVersion:
            {
              m_log->print( __FILE__, __LINE__, 1, esp_get_idf_version(), "SDK: version");
            }
            break;
          case SE_CC_numTasks:
            t1 = uxTaskGetNumberOfTasks();
            pushParameterStack( t1);
            break;
          case SE_CC_cpuPerf:
            t1 = popParameterStack();
            print_real_time_stats(t1, m_log);
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
    snprintf(buf, 5, "%3d", v);
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