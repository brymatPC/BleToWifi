#include "YRShellEsp32.h"
#include "WifiConnection.h"
#include "HttpExecServer.h"
#include "LedBlink.h"
#include <core/IntervalTimer.h>
#include "TelnetServer.h"
#include "UploadDataClient.h"
#include <Preferences.h>
#include <BleConnection.h>
#include "TempHumidityParser.h"
#include "VictronDevice.h"
#include "AppManager.h"

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

#define LED_PIN 21

#define YRSHELL_ON_TELNET
#define LOCAL_LOG_BUFFER_SIZE 8192

static char s_appName[] = "ESP32 BLE Test";
static char s_appVersion[] = "0.9.0";
static const char* TAG = "Main";

Preferences pref;
CircularQ<char, LOCAL_LOG_BUFFER_SIZE> m_logQ;
AppManager appMgr(s_appName, s_appVersion);
YRShellEsp32 shell;
#ifndef HAS_LED_STRIP
  LedBlink onBoardLed;
  LedDriver* ledDriver = &onBoardLed;
#else
  LedStripDriver ledStrip;
  LedDriver* ledDriver = &ledStrip;
#endif
WifiConnection wifiConnection(ledDriver);
HttpExecServer httpServer;
TelnetServer telnetServer;
TelnetLogServer telnetLogServer;
UploadDataClient uploadClient;
BleConnection bleConnection;
VictronDevice victronParser;
TempHumidityParser tempHumParser;

esp_reset_reason_t resetReasonStartup;

void timeSyncNotification(struct timeval *tv) {
    ESP_LOGI(TAG, "Time synchronization event");
}

void startSntp(void) {
    esp_err_t err;
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    config.sync_cb = timeSyncNotification;
    err = esp_netif_sntp_init(&config);
    if(err != ESP_OK) {
      ESP_LOGW(TAG, "SNTP error: %u", err);
    } else {
      ESP_LOGI(TAG, "NTP request started");
    }
}

void preSleepNotification(void) {
    bleConnection.off();
    wifiConnection.off();
}

bool sleepReady(void) {
   return bleConnection.isOff() && wifiConnection.isOff();
}

bool logOut(char c) {
  static char logOverflow[] = "\r\n\nLOG DATA DROPPED\r\n\n";
  bool ret = true;
    if( m_logQ.spaceAvailable( 24)) {
      m_logQ.put( c);
    } else {
      char *s = logOverflow;
      ret = false;
      m_logQ.reset();
      while( *s != '\0') {
        m_logQ.put( *s++);
      }
    }
    return ret;
}
int custom_log_handler(const char* format, va_list args) {
    // Format the message into a buffer
    char buf[128];
    int ret = vsnprintf(buf, sizeof(buf), format, args);
    char *s = buf;
    while( *s != '\0') {
      if(!logOut( *s++)) {
        break;
      }
    }
    return ret; 
}

static void log_char(char c) {
  logOut(c);
}

void setup(){
  unsigned httpPort = 80;
  unsigned telnetPort = 23;
  unsigned telnetLogPort = 2023;
  
  // Use these to redirect Arduino logging
  ets_install_putc2(&log_char);
  ets_install_putc1(NULL);  // closes UART log output
  // Use this to redirect Espressif logging (If enabled)
  //esp_log_set_vprintf(custom_log_handler);


  esp_log_level_set("*", ESP_LOG_WARN);
  esp_log_level_set("AppMgr", ESP_LOG_INFO);
  esp_log_level_set("WifiCon", ESP_LOG_INFO);
  esp_log_level_set("HttpS", ESP_LOG_WARN);
  esp_log_level_set("TelnetS", ESP_LOG_WARN);
  esp_log_level_set("BleCon", ESP_LOG_INFO);
  esp_log_level_set("LedStr", ESP_LOG_WARN);
  esp_log_level_set("Upload", ESP_LOG_WARN);
  esp_log_level_set("THParse", ESP_LOG_WARN);
  esp_log_level_set("Victron", ESP_LOG_WARN);

  resetReasonStartup = esp_reset_reason();

  // BAM - 20260108 - ESP32 uses a USB interface and doesn't support different baud rates
  Serial.begin( 115200);
  // Use this with basic Arduino logging
  //Serial.setDebugOutput(true);

  if(!LittleFS.begin()) {
    ESP_LOGW(TAG, "Could not mount filesystem");
  } else {
    ESP_LOGD(TAG, "Mounted filesystem");
  }

  appMgr.init(pref);
  appMgr.setPreSleepCallback(preSleepNotification);
  appMgr.setSleepReadyCallback(sleepReady);

  #ifndef HAS_LED_STRIP
  onBoardLed.setLedPin( LED_PIN);
#else
  ledStrip.setup();
#endif

  wifiConnection.setup(pref);
  wifiConnection.enable();

  if( httpPort != 0) {
    httpServer.init( httpPort);
    httpServer.setYRShell(&shell);
  }
#ifdef YRSHELL_ON_TELNET
  if( telnetPort != 0) {
    telnetServer.init( telnetPort, &shell.getInq(), &shell.getOutq());
  }
#endif
  if( telnetLogPort != 0) {
    telnetLogServer.init( telnetLogPort);
  }

  uploadClient.init();
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
  shell.setAppMgr(&appMgr);
  shell.setBleConnection(&bleConnection);
  shell.setVictronDevice(&victronParser);
  shell.setTempHumParser(&tempHumParser);
#ifdef HAS_LED_STRIP
  shell.setLedStrip(&ledStrip);
#endif
  victronParser.setup(pref);
  victronParser.setUploadClient(&uploadClient);
  tempHumParser.setUploadClient(&uploadClient);
  shell.init();

  startSntp();
  ESP_LOGD(TAG, "Setup complete");
}


void loop() {
  Sliceable::sliceAll( );

  bool telnetSpaceAvailable = telnetLogServer.spaceAvailable( 32);
  bool serialSpaceAvailable = (Serial.availableForWrite() > 32);
  if( m_logQ.valueAvailable() && (telnetSpaceAvailable || serialSpaceAvailable)) {
    char c;
    for( uint8_t i = 0; i < 32 && m_logQ.valueAvailable(); i++) {
      c = m_logQ.get();
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
