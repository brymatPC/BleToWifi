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

class BleConnection : public Sliceable, public BLEAdvertisedDeviceCallbacks {
private:
    uint8_t m_state;
    bool m_requestScan;
    DebugLog* m_log;
    BLEScan* m_pBleScan;
    bleDeviceData_t m_devices[MAX_BLE_DEVICE_DATA];
    BleParser *m_parser;
    void changeState( uint8_t state);
protected:
public:
    BleConnection( DebugLog* log = nullptr);
    virtual ~BleConnection() { }
    virtual const char* sliceName( void) { return "BleConnection"; }
    virtual void slice( void);

    void requestScan() { m_requestScan = true; }
    bleDeviceData_t *deviceData(uint8_t index);

    void setParser(BleParser *parser) {m_parser = parser;}

    // BLEAdvertisedDeviceCallbacks
    virtual void onResult(BLEAdvertisedDevice advertisedDevice);
    static void scanComplete(BLEScanResults results);
};

#endif // BleConnection_h
