#ifndef BleConnection_h
#define BleConnection_h

#if defined (ESP32)
  #include <BleDevice.h>
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
  BleParser *parser;
} bleDeviceParser_t;

class BleConnection : public Sliceable, public BLEAdvertisedDeviceCallbacks {
private:
    static const uint16_t s_DEFAULT_SCAN_INTERVAL_MS;
    static const uint16_t s_DEFAULT_SCAN_WINDOW_MS;
    static const uint32_t s_DEFAULT_SCAN_DURATION_SEC;
    static const bool s_DEFAULT_SCAN_ACTIVE;
    static const uint32_t s_DEFAULT_SCAN_TIME_MS;

    uint8_t m_state;
    bool m_requestScan;
    bleLogState m_bleLogState;

    DebugLog* m_log;
    IntervalTimer m_scanTimer;
    BLEScan* m_pBleScan;
    bleDeviceData_t m_devices[MAX_BLE_DEVICE_DATA];
    bleDeviceParser_t m_deviceParsers[MAX_BLE_DEVICES];

    // Scan Configuration
    uint16_t m_scanIntervalMs;
    uint16_t m_scanWindowMs;
    uint32_t m_scanDuration;
    bool m_scanActively;
    uint32_t m_scanStartInterval;

    void changeState( uint8_t state);
protected:
public:
    BleConnection( DebugLog* log = nullptr);
    virtual ~BleConnection() { }
    virtual const char* sliceName( void) { return "BleConnection"; }
    virtual void slice( void);

    // Scan Configuration
    void setScanInterval(uint16_t interval) { m_scanIntervalMs = interval; }
    void setScanWindow(uint16_t window) { m_scanWindowMs = window; }
    void setScanDuration(uint32_t duration) { m_scanDuration = duration; }
    void setScanActively(bool actively) { m_scanActively = actively; }
    void setScanStartInterval(uint32_t interval) { m_scanStartInterval = interval; }

    void requestScan() { m_requestScan = true; }
    bleDeviceData_t *deviceData(uint8_t index);

    void setLogState(bleLogState state) {m_bleLogState = state; }
    void enableBleAddress(uint8_t index, const char *addr, BleParser *parser);
    void disableBleAddress(uint8_t index);

    // BLEAdvertisedDeviceCallbacks
    virtual void onResult(BLEAdvertisedDevice advertisedDevice);
    static void scanComplete(BLEScanResults results);
};

#endif // BleConnection_h
