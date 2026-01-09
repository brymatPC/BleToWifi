#ifndef BleConnection_h
#define BleConnection_h

#if defined (ESP32)
  #include <BleDevice.h>
#else
  #warning "BLE is not supported on the selected target"
#endif
#include <utility/Sliceable.h>
#include <utility/IntervalTimer.h>
#include <utility/DebugLog.h>

class BleConnection : public Sliceable, public BLEAdvertisedDeviceCallbacks {
private:
    uint8_t m_state;
    DebugLog* m_log;
    BLEScan* m_pBleScan;
    void changeState( uint8_t state);
protected:
public:
    BleConnection( DebugLog* log = nullptr);
    virtual ~BleConnection() { }
    virtual const char* sliceName( void) { return "BleConnection"; }
    virtual void slice( void);

    // BLEAdvertisedDeviceCallbacks
    virtual void onResult(BLEAdvertisedDevice advertisedDevice);
    static void scanComplete(BLEScanResults results);
};

#endif // BleConnection_h
