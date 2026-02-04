#include "BleConnection.h"
#include <utility/String.h>

static bool m_resultsReceived = false;
static BLEScanResults m_results;

const uint16_t BleConnection::s_DEFAULT_SCAN_INTERVAL_MS = 1000;
const uint16_t BleConnection::s_DEFAULT_SCAN_WINDOW_MS = 1000;
const uint32_t BleConnection::s_DEFAULT_SCAN_DURATION_SEC = 10;
const bool BleConnection::s_DEFAULT_SCAN_ACTIVE = false;
const uint32_t BleConnection::s_DEFAULT_SCAN_TIME_MS = 0;

typedef enum {
  STATE_RESET      = 0,
  STATE_START_SCAN = 1,
  STATE_IDLE       = 2,
  STATE_SCANNING   = 3,

} bleStates_t;

BleConnection::BleConnection( DebugLog* log)
{
    m_log = log;
    m_state = STATE_RESET;
    m_requestScan = false;
    m_bleLogState = BLE_LOG_NONE;

    m_scanIntervalMs = s_DEFAULT_SCAN_INTERVAL_MS;
    m_scanWindowMs = s_DEFAULT_SCAN_WINDOW_MS;
    m_scanDuration = s_DEFAULT_SCAN_DURATION_SEC;
    m_scanActively = s_DEFAULT_SCAN_ACTIVE;
    m_scanStartInterval = s_DEFAULT_SCAN_TIME_MS;
    m_scanTimer.setInterval(s_DEFAULT_SCAN_TIME_MS);

    for(uint8_t i=0; i < MAX_BLE_DEVICES; i++) {
        m_deviceParsers[i].enabled = false;
    }
}

void BleConnection::enableBleAddress(uint8_t index, const char *addr, BleParser *parser) {
    if(index >= MAX_BLE_DEVICES) return;
    strncpy(m_deviceParsers[index].addr, addr, BLE_ADDR_LEN);
    m_deviceParsers[index].parser = parser;
    m_deviceParsers[index].enabled = true;
}
void BleConnection::disableBleAddress(uint8_t index) {
    if(index >= MAX_BLE_DEVICES) return;
    m_deviceParsers[index].enabled = false;
}
void BleConnection::setBleAddress(uint8_t index, const char *addr) {
    if(index >= MAX_BLE_DEVICES) return;
    strncpy(m_deviceParsers[index].addr, addr, BLE_ADDR_LEN);
}
void BleConnection::setBleParser(uint8_t index, BleParser *parser) {
    if(index >= MAX_BLE_DEVICES) return;
    m_deviceParsers[index].parser = parser;
}
void BleConnection::setBleEnable(uint8_t index, bool enable) {
    if(index >= MAX_BLE_DEVICES) return;
    m_deviceParsers[index].enabled = enable;
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
            changeState( STATE_IDLE);
        break;
        case STATE_IDLE:
            if( m_requestScan) {
                m_requestScan = false;
                if( m_log) {
                    m_log->print( __FILE__, __LINE__, 1, "BleConnection: scan requested");
                }
                changeState( STATE_START_SCAN);
            } else if(m_scanStartInterval != 0 && m_scanTimer.hasIntervalElapsed()) {
                m_scanTimer.setInterval(m_scanStartInterval);
                if( m_log) {
                    m_log->print( __FILE__, __LINE__, 1, "BleConnection: scan start interval");
                }
                changeState( STATE_START_SCAN);
            }
        break;
        case STATE_START_SCAN:
            m_resultsReceived = false;
            m_pBleScan->setInterval(m_scanIntervalMs);
            m_pBleScan->setWindow(m_scanWindowMs);
            m_pBleScan->setActiveScan(m_scanActively);
            m_pBleScan->start(m_scanDuration, scanComplete);
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
    BLEAddress address = advertisedDevice.getAddress();
    for(uint8_t i=0; i < MAX_BLE_DEVICES; i++) {
        if(!m_deviceParsers[i].enabled) continue;
        BLEAddress addrToParse(m_deviceParsers[i].addr);
        if(address == addrToParse) {
            m_devices[0].payloadLen = 0;
            strcpy(m_devices[0].addr, address.toString().c_str());
            if (advertisedDevice.haveManufacturerData()) {
                int len = advertisedDevice.getManufacturerData().length();
                const char* data = advertisedDevice.getManufacturerData().c_str();
                memcpy(m_devices[0].payload, data, len);
                m_devices[0].payloadLen = (uint8_t) len;
                m_devices[0].valid = true;

                if(m_deviceParsers[i].parser) {
                    m_deviceParsers[i].parser->setData(m_devices[0]);
                    m_deviceParsers[i].parser->parse();
                }
            }
        }
    }

    if(m_log && m_bleLogState != BLE_LOG_NONE) {
        m_log->print( __FILE__, __LINE__, 1, address.toString().c_str(), "BleConnection: address");
        if (advertisedDevice.haveName()) {
            m_log->print( __FILE__, __LINE__, 1, advertisedDevice.getName().c_str(), "BleConnection: name");
        }

        if(m_bleLogState == BLE_LOG_ALL) {
            if (advertisedDevice.haveRSSI()) {
                int rssi = advertisedDevice.getRSSI();
                rssi *= -1;
                m_log->print( __FILE__, __LINE__, 1, rssi, "BleConnection: -RSSI");
            }
            if (advertisedDevice.haveTXPower()) {
                m_log->print( __FILE__, __LINE__, 1, advertisedDevice.getTXPower(), "BleConnection: TxPower");
            }
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
                    if( m_log) {
                        m_log->print( __FILE__, __LINE__, 1, outStr, "BleConnection: mfdata");
                    }
                #endif
            }
            m_log->print( __FILE__, __LINE__, 1, advertisedDevice.getPayloadLength(), "BleConnection: payloadLen");

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