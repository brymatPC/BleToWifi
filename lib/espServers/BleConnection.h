#ifndef BleConnection_h
#define BleConnection_h

#if defined (ESP32)
  #include <BleDevice.h>
  #include <Preferences.h>
#else
  #warning "BLE is not supported on the selected target"
#endif

#include "BleParser.h"

#include <utility/Sliceable.h>
#include <utility/IntervalTimer.h>
#include <utility/DebugLog.h>

#define MAX_BLE_DEVICE_DATA 1
#define MAX_BLE_DEVICES     4
#define BLE_ADDR_LEN        20

typedef enum {
  BLE_LOG_NONE,
  BLE_LOG_ADDRESSES,
  BLE_LOG_ALL
} bleLogState;

typedef struct {
  bool enabled;
  char addr[BLE_ADDR_LEN];
  BleParserTypes parserType;
} bleDeviceParser_t;

class BleConnection : public Sliceable, public BLEAdvertisedDeviceCallbacks {
private:
    static const char s_PREF_NAMESPACE[];
    static const uint16_t s_DEFAULT_SCAN_INTERVAL_MS;
    static const uint16_t s_DEFAULT_SCAN_WINDOW_MS;
    static const uint32_t s_DEFAULT_SCAN_DURATION_SEC;
    static const bool s_DEFAULT_SCAN_ACTIVE;
    static const uint32_t s_DEFAULT_SCAN_TIME_MS;
    static const uint32_t s_DEFAULT_SCAN_BOOT_MS;

    static const char parserKeyEnPrefix[];
    static const char parserKeyAddrPrefix[];
    static const char parserKeyParserPrefix[];

    uint8_t m_state;
    bool m_requestScan;
    bleLogState m_bleLogState;

    DebugLog* m_log;
    IntervalTimer m_scanTimer;
    BLEScan* m_pBleScan;
    bleDeviceData_t m_devices[MAX_BLE_DEVICE_DATA];
    bleDeviceParser_t m_deviceParsers[MAX_BLE_DEVICES];
    BleParserTypes m_parserTypes[MAX_BLE_DEVICES];
    BleParser* m_parsers[MAX_BLE_DEVICES];
    uint8_t m_nextParser;

    // Scan Configuration
    uint16_t m_scanIntervalMs;
    uint16_t m_scanWindowMs;
    uint32_t m_scanDuration;
    bool m_scanActively;
    uint32_t m_scanStartInterval;
    uint32_t m_scanStartBoot;

    void changeState( uint8_t state);
    BleParser* getParser(BleParserTypes type);
protected:
public:
    BleConnection(DebugLog* log = nullptr);
    virtual ~BleConnection() { }
    virtual const char* sliceName( void) { return "BleConnection"; }
    virtual void slice( void);

    void setup(Preferences &pref);
    void save(Preferences &pref);

    void addParser(BleParserTypes type, BleParser *parser);

    // Scan Configuration
    void setScanInterval(uint16_t interval) { m_scanIntervalMs = interval; }
    void setScanWindow(uint16_t window) { m_scanWindowMs = window; }
    void setScanDuration(uint32_t duration) { m_scanDuration = duration; }
    void setScanActively(bool actively) { m_scanActively = actively; }
    void setScanStartInterval(uint32_t interval) { m_scanStartInterval = interval; }
    void setScanStartBoot(uint32_t interval) { m_scanStartBoot = interval; }

    void requestScan() { m_requestScan = true; }
    void off();
    bleDeviceData_t *deviceData(uint8_t index);

    void setLogState(bleLogState state) {m_bleLogState = state; }
    void setBleAddress(uint8_t index, const char *addr);
    void setBleParser(uint8_t index, BleParserTypes type);
    void setBleEnable(uint8_t index, bool enable);
    void logParsers();

    // BLEAdvertisedDeviceCallbacks
    virtual void onResult(BLEAdvertisedDevice advertisedDevice);
    static void scanComplete(BLEScanResults results);
};

#endif // BleConnection_h
