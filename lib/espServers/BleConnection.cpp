#include "BleConnection.h"

#define SCAN_INTERVAL_MILLIS 625
#define SCAN_WINDOW_MILLIS 625
#define SCAN_DURATION_SECS 30
#define SCAN_ACTIVELY false

static bool m_resultsReceived = false;
static BLEScanResults m_results;

typedef enum {
  STATE_RESET             = 0,
  STATE_START_SCAN      = 1,
  STATE_IDLE      = 2,

} bleStates_t;

BleConnection::BleConnection( DebugLog* log) {
    m_log = log;
    m_state = STATE_RESET;
}

void BleConnection::changeState( uint8_t state) {
    if( m_log) {
        m_log->print( __FILE__, __LINE__, 1, m_state, state, "BleConnection::changeState: m_state, state" );
    }
    m_state = state;
}

void BleConnection::slice( void) {

    switch( m_state) {
        case STATE_RESET:
            BLEDevice::init("ble_esp32");
            m_pBleScan = BLEDevice::getScan();
            m_pBleScan->setAdvertisedDeviceCallbacks(this, false);
            m_pBleScan->setInterval(SCAN_INTERVAL_MILLIS);
            m_pBleScan->setWindow(SCAN_WINDOW_MILLIS);
            m_pBleScan->setActiveScan(SCAN_ACTIVELY);
            changeState( STATE_START_SCAN);
        break;
        case STATE_START_SCAN:
            m_resultsReceived = false;
            m_pBleScan->start(SCAN_DURATION_SECS, scanComplete);
            changeState( STATE_IDLE);
        break;
        case STATE_IDLE:
            if( m_resultsReceived) {
                if( m_log) {
                    m_log->print( __FILE__, __LINE__, 1, m_results.getCount(), "BleConnection: scan result count");
                }
                m_resultsReceived = false;
                //changeState( STATE_START_SCAN);
            }
        break;
    }
}

void BleConnection::onResult(BLEAdvertisedDevice advertisedDevice) {
    if( m_log) {
        BLEAddress address = advertisedDevice.getAddress();
        m_log->print( __FILE__, __LINE__, 1, address.toString().c_str(), "BleConnection: address");
        if (advertisedDevice.haveName()) {
            //Serial.printf(" %10s", advertisedDevice.getName().c_str());
            m_log->print( __FILE__, __LINE__, 1, advertisedDevice.getName().c_str(), "BleConnection: name");
        }
        if (advertisedDevice.haveRSSI()) {
            //Serial.printf(" RSSI=%d", advertisedDevice.getRSSI());
            m_log->print( __FILE__, __LINE__, 1, advertisedDevice.getRSSI(), "BleConnection: RSSI");
        }
        if (advertisedDevice.haveTXPower()) {
            //Serial.printf(" TXP=%d", advertisedDevice.getTXPower());
            m_log->print( __FILE__, __LINE__, 1, advertisedDevice.getTXPower(), "BleConnection: TxPower");
        }
        // if (advertisedDevice.haveAppearance()) {
        //     Serial.printf(" App=0x%04x", advertisedDevice.getAppearance());
        // }
        if (advertisedDevice.haveManufacturerData()) {
            //Serial.printf(" %s", advertisedDevice.getManufacturerData().c_str());
            m_log->print( __FILE__, __LINE__, 1, advertisedDevice.getName().c_str(), "BleConnection: manufacturer");
        }

        // uint8_t *block = advertisedDevice.getPayload();
        // uint8_t *endData = block + advertisedDevice.getPayloadLength();
        // while (block < endData) {
        //     block = parseBlock(block, endData);
        // }

        if (advertisedDevice.getServiceUUIDCount() > 0) {
            //Serial.printf("  Services(%d):\n", advertisedDevice.getServiceUUIDCount());
            m_log->print( __FILE__, __LINE__, 1, advertisedDevice.getServiceUUIDCount(), "BleConnection: Service UUID count");
            // for (int i = 0; i < advertisedDevice.getServiceUUIDCount(); i++) {
            //     Serial.printf("    %s\n", advertisedDevice.getServiceUUID(i).toString().c_str());
            // }
        }

        if (advertisedDevice.getServiceDataUUIDCount()) {
            //Serial.printf("  ServiceData(%d):\n", advertisedDevice.getServiceDataUUIDCount());
            m_log->print( __FILE__, __LINE__, 1, advertisedDevice.getServiceDataUUIDCount(), "BleConnection: Service data UUID count");
            // for (int i = 0; i < advertisedDevice.getServiceDataUUIDCount(); i++) {
            //     std::string data = advertisedDevice.getServiceData(i);
            //     Serial.printf("    %s %3d ", advertisedDevice.getServiceDataUUID(i).toString().c_str(), data.length());
            //     printBuffer((uint8_t*)data.c_str(), data.length());
            //     Serial.println();
            // }
        }
    }
}

void BleConnection::scanComplete(BLEScanResults results) {
    m_results = results;
    m_resultsReceived = true;
}