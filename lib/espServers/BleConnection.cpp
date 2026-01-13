#include "BleConnection.h"
#include <utility/String.h>

#define SCAN_INTERVAL_MILLIS 625
#define SCAN_WINDOW_MILLIS 625
#define SCAN_DURATION_SECS 20
#define SCAN_ACTIVELY false

static bool m_resultsReceived = false;
static BLEScanResults m_results;

static BLEAddress s_VictronAddr("df:3e:15:b9:42:9e");

typedef enum {
  STATE_RESET      = 0,
  STATE_START_SCAN = 1,
  STATE_IDLE       = 2,
  STATE_SCANNING   = 3,

} bleStates_t;

BleConnection::BleConnection( DebugLog* log) {
    m_log = log;
    m_state = STATE_RESET;
    m_requestScan = false;
    m_parser = nullptr;
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
        case STATE_IDLE:
            if( m_requestScan) {
                m_requestScan = false;
                if( m_log) {
                    m_log->print( __FILE__, __LINE__, 1, "BleConnection: scan requested");
                }
                changeState( STATE_START_SCAN);
            }
        break;
        case STATE_START_SCAN:
            m_resultsReceived = false;
            m_pBleScan->start(SCAN_DURATION_SECS, scanComplete);
            changeState( STATE_SCANNING);
        break;
        case STATE_SCANNING:
            if( m_resultsReceived) {
                if( m_log) {
                    m_log->print( __FILE__, __LINE__, 1, m_results.getCount(), "BleConnection: scan result count");
                }
                m_resultsReceived = false;
                changeState( STATE_IDLE);
            }
        break;
    }
}

void BleConnection::onResult(BLEAdvertisedDevice advertisedDevice) {
    if( m_log) {
        BLEAddress address = advertisedDevice.getAddress();
        m_log->print( __FILE__, __LINE__, 1, address.toString().c_str(), "BleConnection: address");
        
        if (advertisedDevice.haveName()) {
            m_log->print( __FILE__, __LINE__, 1, advertisedDevice.getName().c_str(), "BleConnection: name");
        }

        if(address == s_VictronAddr) {
            m_log->print( __FILE__, __LINE__, 1, "BleConnection: Found Victron");
            m_devices[0].payloadLen = 0;
            strcpy(m_devices[0].addr, address.toString().c_str());
            if (advertisedDevice.haveRSSI()) {
                int rssi = advertisedDevice.getRSSI();
                rssi *= -1;
                m_log->print( __FILE__, __LINE__, 1, rssi, "BleConnection: -RSSI");
            }
            if (advertisedDevice.haveTXPower()) {
                m_log->print( __FILE__, __LINE__, 1, advertisedDevice.getTXPower(), "BleConnection: TxPower");
            }
            // if (advertisedDevice.haveAppearance()) {
            //     Serial.printf(" App=0x%04x", advertisedDevice.getAppearance());
            // }
            if (advertisedDevice.haveManufacturerData()) {
                int len = advertisedDevice.getManufacturerData().length();
                const char* data = advertisedDevice.getManufacturerData().c_str();
                #ifdef LOG_INPUT_DATA
                    char outStr[128];
                    outStr[0] = '0';
                    outStr[1] = 'x';
                    for(int i=0; i < len; i++) {
                        sprintf(&outStr[2+i*2], "%02X", data[i]);
                    }
                    m_log->print( __FILE__, __LINE__, 1, outStr, "BleConnection: mfdata");
                #endif

                memcpy(m_devices[0].payload, data, len);
                m_devices[0].payloadLen = (uint8_t) len;
                m_devices[0].valid = true;

                if(m_parser) {
                    m_parser->setData(m_devices[0]);
                    m_parser->parse();
                }
            }

            m_log->print( __FILE__, __LINE__, 1, advertisedDevice.getPayloadLength(), "BleConnection: payloadLen");

            // uint8_t *block = advertisedDevice.getPayload();
            // uint8_t *endData = block + advertisedDevice.getPayloadLength();
            // while (block < endData) {
            //     block = parseBlock(block, endData);
            // }

            // if (advertisedDevice.getServiceUUIDCount() > 0) {
            //     m_log->print( __FILE__, __LINE__, 1, advertisedDevice.getServiceUUIDCount(), "BleConnection: Service UUID count");
            //     for (int i = 0; i < advertisedDevice.getServiceUUIDCount(); i++) {
            //         m_log->print( __FILE__, __LINE__, 1, i, advertisedDevice.getServiceUUID(i).toString().c_str(), "BleConnection: i, Service UUID");
            //     }
            // }

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
}

void BleConnection::scanComplete(BLEScanResults results) {
    m_results = results;
    m_resultsReceived = true;
}

bleDeviceData_t *BleConnection::deviceData(uint8_t index) {
    if(index >= MAX_BLE_DEVICE_DATA) {
        return nullptr;
    }
    return &m_devices[index];
}