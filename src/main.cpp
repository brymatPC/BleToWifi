#include "YRShellEsp32.h"
#include "WifiConnection.h"
#include "HttpExecServer.h"
#include <utility/LedBlink.h>
#include <utility/IntervalTimer.h>
#include "TelnetServer.h"
#include "UploadDataClient.h"
#include <Preferences.h>
#include <BleConnection.h>
#include "TempHumidityParser.h"
#include "VictronDevice.h"

#ifdef HAS_LED_STRIP
  #include "LedStripDriver.h"
#endif

#include "esp_netif_sntp.h"
//#include "esp_sntp.h"

//  0x01 - setup log
//  0x02 - errors
//  0x04 - exec output
//  0x08 - file load output
 
//  0x0100 - telnet long slice
//  0x0200 - telnet connect / disconnect
//  0x0400 - telnet char received
//  0x0800 - telnet control
//  0x1000 - telnet state change

// 0x010000 - http long slice
// 0x020000 - http connect / disconnect
// 0x040000 - http request
// 0x080000 - http request process
// 0x100000 - http state change
// 0x200000 - http log

// 0x80000000 - YRShellInterpreter debug

//#define LOG_MASK 0x80331303
#define LOG_MASK 0x80000003

#define STATUS_TIME_INTERVAL_MS (60000)

#define LED_PIN 21

#define YRSHELL_ON_TELNET

Preferences pref;
DebugLog dbg;
YRShellEsp32 shell;
#ifndef HAS_LED_STRIP
  LedBlink onBoardLed;
  LedDriver* ledDriver = &onBoardLed;
#else
  LedStripDriver ledStrip;
  LedDriver* ledDriver = &ledStrip;
#endif
WifiConnection wifiConnection(ledDriver, &dbg);
HttpExecServer httpServer;
TelnetServer telnetServer;
TelnetLogServer telnetLogServer;
UploadDataClient uploadClient;
BleConnection bleConnection(&dbg);
VictronDevice victronParser;
TempHumidityParser tempHumParser;

IntervalTimer systemStatusTimer;
esp_reset_reason_t resetReasonStartup;

static char s_appName[] = "ESP32 BLE Test";
static char s_appVersion[] = "0.9.0";

static char s_resetUnknownStr[]     = "unknown";
static char s_resetPowerOnStr[]     = "power-on";
static char s_resetExtStr[]         = "external";
static char s_resetSoftwareStr[]    = "Software";
static char s_resetPanicStr[]       = "panic";
static char s_resetIntWdogStr[]     = "interrupt watchdog";
static char s_resetTaskWdogStr[]    = "task watchdog";
static char s_resetOthWdogStr[]     = "other watchdog";
static char s_resetDeepSleepStr[]   = "deep sleep";
static char s_resetBrownOutStr[]    = "brown out";
static char s_resetSDIOStr[]        = "sdio";
static char s_resetUsbStr[]         = "usb";
static char s_resetJtagStr[]        = "jtag";
static char s_resetEFuseStr[]       = "efuse";
static char s_resetPowerGlitchStr[] = "power glitch";
static char s_resetCpuLockupStr[]   = "cpu lockup";

void timeSyncNotification(struct timeval *tv) {
    dbg.print(__FILE__, __LINE__, 1, "Notification of a time synchronization event");
}

void startSntp(void) {
    esp_err_t err;
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    config.sync_cb = timeSyncNotification;
    err = esp_netif_sntp_init(&config);
    if(err != ESP_OK) {
      dbg.print(__FILE__, __LINE__, 1, err, "startSntp:init - error");
    } else {
      dbg.print(__FILE__, __LINE__, 1, "startSntp - ntp request started");
    }
}

char *resetReasonToString(esp_reset_reason_t reason) {
  char *result;
  switch (reason) {
  case ESP_RST_POWERON:
      result = s_resetPowerOnStr;
      break;
  case ESP_RST_EXT:
      result = s_resetExtStr;
      break;
  case ESP_RST_SW:
      result = s_resetSoftwareStr;
      break;
  case ESP_RST_PANIC:
      result = s_resetPanicStr;
      break;
  case ESP_RST_INT_WDT:
      result = s_resetIntWdogStr;
      break;
  case ESP_RST_TASK_WDT:
      result = s_resetTaskWdogStr;
      break;
  case ESP_RST_WDT:
      result = s_resetOthWdogStr;
      break;
  case ESP_RST_DEEPSLEEP:
      result = s_resetDeepSleepStr;
      break;
  case ESP_RST_BROWNOUT:
      result = s_resetBrownOutStr;
      break;
  case ESP_RST_SDIO:
      result = s_resetSDIOStr;
      break;
  case ESP_RST_USB:
      result = s_resetUsbStr;
      break;
  case ESP_RST_JTAG:
      result = s_resetJtagStr;
      break;
  case ESP_RST_EFUSE:
      result = s_resetEFuseStr;
      break;
  case ESP_RST_PWR_GLITCH:
      result = s_resetPowerGlitchStr;
      break;
  case ESP_RST_CPU_LOCKUP:
      result = s_resetCpuLockupStr;
      break;
  case ESP_RST_UNKNOWN:
  default:
      result = s_resetUnknownStr;
      break;
  }
  return result;
}

void setup(){
  unsigned httpPort = 80;
  unsigned telnetPort = 23;
  unsigned telnetLogPort = 2023;
  dbg.setMask( LOG_MASK);

  resetReasonStartup = esp_reset_reason();

  // BAM - 20260108 - ESP32 uses a USB interface and doesn't support different baud rates
  BSerial.begin( 115200);

  dbg.print( __FILE__, __LINE__, 1, "\r\n\n");

  if(!LittleFS.begin()) {
    dbg.print( __FILE__, __LINE__, 1, "setup_Could_not_mount_file_system:");
  } else {
    dbg.print( __FILE__, __LINE__, 1, "setup_Mounted_file_system:");
  }

  systemStatusTimer.setInterval(STATUS_TIME_INTERVAL_MS);

  #ifndef HAS_LED_STRIP
  onBoardLed.setLedPin( LED_PIN);
#else
  ledStrip.setup(&dbg);
#endif

  wifiConnection.setup(pref);
  wifiConnection.enable();

  if( httpPort != 0) {
    httpServer.init( httpPort, &dbg);
    httpServer.setYRShell(&shell);
  }
#ifdef YRSHELL_ON_TELNET
  if( telnetPort != 0) {
    telnetServer.init( telnetPort, &shell.getInq(), &shell.getOutq(), &dbg);
  }
#endif
  if( telnetLogPort != 0) {
    telnetLogServer.init( telnetLogPort);
  }

  uploadClient.init(&dbg);
  uploadClient.setup(pref);

#ifndef YRSHELL_ON_TELNET
  BSerial.init(shell.getInq(), shell.getOutq());
#endif

  shell.setLedDriver(ledDriver);
  shell.setWifiConnection(&wifiConnection);
  shell.setTelnetLogServer(&telnetLogServer);
  shell.setUploadClient(&uploadClient);
  bleConnection.setup(pref);
  bleConnection.addParser(BleParserTypes::victron, &victronParser);
  bleConnection.addParser(BleParserTypes::tempHumidity, &tempHumParser);
  shell.setPreferences(&pref);
  shell.setBleConnection(&bleConnection);
  shell.setVictronDevice(&victronParser);
  shell.setTempHumParser(&tempHumParser);
#ifdef HAS_LED_STRIP
  shell.setLedStrip(&ledStrip);
#endif
  victronParser.setup(pref);
  victronParser.init(&dbg);
  victronParser.setUploadClient(&uploadClient);
  tempHumParser.init(&dbg);
  tempHumParser.setUploadClient(&uploadClient);
  shell.init( &dbg);

  startSntp();
  dbg.print( __FILE__, __LINE__, 1, "setup_done:");
}


void loop() {
  Sliceable::sliceAll( );

  if(systemStatusTimer.isNextInterval()) {
    dbg.print(__FILE__, __LINE__, 1, "System Status Only, not a reboot!");
    dbg.print(__FILE__, __LINE__, 1, s_appName, "s_appName");
    dbg.print(__FILE__, __LINE__, 1, s_appVersion, "s_appVersion");
    dbg.print(__FILE__, __LINE__, 1, resetReasonStartup, resetReasonToString(resetReasonStartup), "Reset Reason - resetReasonStartup, resetReasonStartupStr");
  }

  bool telnetSpaceAvailable = telnetLogServer.spaceAvailable( 32);
  bool serialSpaceAvailable = (Serial.availableForWrite() > 32);
  if( dbg.valueAvailable() && (telnetSpaceAvailable || serialSpaceAvailable)) {
    char c;
    for( uint8_t i = 0; i < 32 && dbg.valueAvailable(); i++) {
      c = dbg.get();
      if(telnetSpaceAvailable) {
        telnetLogServer.put( c);
      }
#ifdef YRSHELL_ON_TELNET
      if(serialSpaceAvailable) {
        Serial.print( c );
      }
#endif
    }
  }
}
