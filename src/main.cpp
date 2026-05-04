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
#include <Wire.h>
#include <SensirionI2cSen66.h>
#include "Sen66Device.h"

#include <SPI.h>
#include <SD.h>

#ifdef HAS_LED_STRIP
  #include "LedStripDriver.h"
#endif

#include "esp_netif_sntp.h"
//#include "esp_sntp.h"

#include "esp_log_custom.h"

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

#define I2C_SDA_PIN 1
#define I2C_SCL_PIN 2

static char s_appName[] = "ESP32 BLE Test";
static char s_appVersion[] = "0.9.0";
static const char* TAG = "Main   ";

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

SensirionI2cSen66 sensor;
Sen66Device sen66Device(sensor);

// For SD Card access
SPIClass sd_spi(HSPI);

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

void testFileIO(fs::FS &fs, const char * path){
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file){
      len = file.size();
      size_t flen = len;
      start = millis();
      while(len){
          size_t toRead = len;
          if(toRead > 512){
              toRead = 512;
          }
          file.read(buf, toRead);
          len -= toRead;
      }
      end = millis() - start;
      ESP_LOGI(TAG, "%u bytes read in %u ms", flen, end);
      file.close();
  } else {
      ESP_LOGI(TAG, "Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if(!file){
      ESP_LOGI(TAG, "Failed to open file for writing");
      return;
  }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++){
      file.write(buf, 512);
  }
  end = millis() - start;
  ESP_LOGI(TAG, "%u bytes written in %u ms", 2048 * 512, end);
  file.close();
}

void testSdCard() {
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
      ESP_LOGI(TAG, "No SD card attached");
      return;
    }

    ESP_LOGI(TAG, "SD Card Type: ");
    if(cardType == CARD_MMC){
      ESP_LOGI(TAG, "MMC");
    } else if(cardType == CARD_SD){
      ESP_LOGI(TAG, "SDSC");
    } else if(cardType == CARD_SDHC){
      ESP_LOGI(TAG, "SDHC");
    } else {
      ESP_LOGI(TAG, "UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    ESP_LOGI(TAG, "SD Card Size: %lluMB\n", cardSize);

    // listDir(SD, "/", 0);
    // createDir(SD, "/mydir");
    // listDir(SD, "/", 0);
    // removeDir(SD, "/mydir");
    // listDir(SD, "/", 2);
    // writeFile(SD, "/hello.txt", "Hello ");
    // appendFile(SD, "/hello.txt", "World!\n");
    // readFile(SD, "/hello.txt");
    // deleteFile(SD, "/foo.txt");
    // renameFile(SD, "/hello.txt", "/foo.txt");
    // readFile(SD, "/foo.txt");
    testFileIO(SD, "/test.txt");
    ESP_LOGI(TAG, "Total space: %lluMB", SD.totalBytes() / (1024 * 1024));
    ESP_LOGI(TAG, "Used space: %lluMB", SD.usedBytes() / (1024 * 1024));
}

void setup(){
  unsigned httpPort = 80;
  unsigned telnetPort = 23;
  unsigned telnetLogPort = 2023;
  
  // Use these to redirect Arduino logging
  // ets_install_putc2(&log_char);
  // ets_install_putc1(NULL);  // closes UART log output
  // Use this to redirect Espressif logging (If enabled)
  esp_log_set_vprintf(custom_log_handler);


  esp_log_level_set("*", ESP_LOG_WARN);
  esp_log_level_set("Main   ", ESP_LOG_INFO);
  esp_log_level_set("AppMgr ", ESP_LOG_INFO);
  esp_log_level_set("YRShell", ESP_LOG_INFO);
  esp_log_level_set("WifiCon", ESP_LOG_DEBUG);
  esp_log_level_set("HttpS  ", ESP_LOG_WARN);
  esp_log_level_set("TelnetS", ESP_LOG_WARN);
  esp_log_level_set("BleCon ", ESP_LOG_INFO);
  esp_log_level_set("LedStr ", ESP_LOG_WARN);
  esp_log_level_set("Upload ", ESP_LOG_WARN);
  esp_log_level_set("THParse", ESP_LOG_INFO);
  esp_log_level_set("Victron", ESP_LOG_INFO);
  esp_log_level_set("Sen66  ", ESP_LOG_WARN);

  resetReasonStartup = esp_reset_reason();

  // BAM - 20260108 - ESP32 uses a USB interface and doesn't support different baud rates
  Serial.begin( 115200);
  // Use this with basic Arduino logging
  //Serial.setDebugOutput(true);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

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

  sensor.begin(Wire, SEN66_I2C_ADDR_6B);
  sen66Device.setup(pref);
  sen66Device.setUploadClient(&uploadClient);

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
  shell.setSen66Device(&sen66Device);
#ifdef HAS_LED_STRIP
  shell.setLedStrip(&ledStrip);
#endif
  victronParser.setup(pref);
  victronParser.setUploadClient(&uploadClient);
  tempHumParser.setUploadClient(&uploadClient);
  shell.init();

  sd_spi.begin(10, 7, 8, 11);
  if(!SD.begin(11, sd_spi)) {
    ESP_LOGW(TAG, "SD Card begin failed");
  } else {
    testSdCard();
  }

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
