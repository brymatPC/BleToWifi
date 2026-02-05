#include "YRShellEsp32.h"
#include "WifiConnection.h"
#include "HttpExecServer.h"
#include <utility/LedBlink.h>
#include "TelnetServer.h"
#include "UploadDataClient.h"
#include <Preferences.h>
#include <BleConnection.h>
#include "TempHumidityParser.h"
#include "VictronDevice.h"

#ifdef HAS_LED_STRIP
  #include "LedStripDriver.h"
#endif

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

void setup(){
  unsigned httpPort = 80;
  unsigned telnetPort = 23;
  unsigned telnetLogPort = 2023;
  dbg.setMask( LOG_MASK);

  // BAM - 20260108 - ESP32 uses a USB interface and doesn't support different baud rates
  BSerial.begin( 115200);

  dbg.print( __FILE__, __LINE__, 1, "\r\n\n");

  if(!LittleFS.begin()) {
    dbg.print( __FILE__, __LINE__, 1, "setup_Could_not_mount_file_system:");
  } else {
    dbg.print( __FILE__, __LINE__, 1, "setup_Mounted_file_system:");
  }

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
  dbg.print( __FILE__, __LINE__, 1, "setup_done:");
}

void loop() {
  Sliceable::sliceAll( );
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
