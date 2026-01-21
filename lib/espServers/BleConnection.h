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

typedef enum {
  BLE_LOG_NONE,
  BLE_LOG_ADDRESSES,
  BLE_LOG_ALL
} bleLogState;

class BleConnection : public Sliceable, public BLEAdvertisedDeviceCallbacks {
private:
    uint8_t m_state;
    bool m_requestScan;
    bleLogState m_bleLogState;

    DebugLog* m_log;
    BLEScan* m_pBleScan;
    bleDeviceData_t m_devices[MAX_BLE_DEVICE_DATA];
    BleParser *m_parser;
    char m_addrToParse[20];
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
    void setParser(BleParser *parser) {m_parser = parser;}
    void setAddressToParse(const char *addr);

    // BLEAdvertisedDeviceCallbacks
    virtual void onResult(BLEAdvertisedDevice advertisedDevice);
    static void scanComplete(BLEScanResults results);
};

#endif // BleConnection_h
