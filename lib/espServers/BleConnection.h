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
    uint8_t m_state;
    bool m_requestScan;
    bleLogState m_bleLogState;

    DebugLog* m_log;
    BLEScan* m_pBleScan;
    bleDeviceData_t m_devices[MAX_BLE_DEVICE_DATA];
    bleDeviceParser_t m_deviceParsers[MAX_BLE_DEVICES];
    void changeState( uint8_t state);
protected:
public:
    BleConnection( DebugLog* log = nullptr);
    virtual ~BleConnection() { }
    virtual const char* sliceName( void) { return "BleConnection"; }
    virtual void slice( void);

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
