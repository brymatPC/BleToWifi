#include "BleConnection.h"
#include <utility/String.h>

static bool m_resultsReceived = false;
static BLEScanResults m_results;

const char BleConnection::s_PREF_NAMESPACE[] = "ble";
const uint16_t BleConnection::s_DEFAULT_SCAN_INTERVAL_MS = 1000;
const uint16_t BleConnection::s_DEFAULT_SCAN_WINDOW_MS = 1000;
const uint32_t BleConnection::s_DEFAULT_SCAN_DURATION_SEC = 10;
const bool BleConnection::s_DEFAULT_SCAN_ACTIVE = false;
const uint32_t BleConnection::s_DEFAULT_SCAN_TIME_MS = 0;

const char BleConnection::parserKeyEnPrefix[] = "parEn";
const char BleConnection::parserKeyAddrPrefix[] = "parAddr";
const char BleConnection::parserKeyParserPrefix[] = "parPar";

typedef enum {
  STATE_RESET      = 0,
  STATE_START_SCAN = 1,
  STATE_IDLE       = 2,
  STATE_SCANNING   = 3,
  STATE_OFF        = 4,

} bleStates_t;

BleConnection::BleConnection(DebugLog* log)
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
        m_parserTypes[i] = BleParserTypes::none;
        m_parsers[i] = nullptr;
    }
    m_nextParser = 0;
}
void BleConnection::setup(Preferences &pref) {
    char parserKey[16];
    pref.begin(s_PREF_NAMESPACE, true);
    m_scanIntervalMs    = pref.getUShort("scanInt", s_DEFAULT_SCAN_INTERVAL_MS);
    m_scanWindowMs      = pref.getUShort("scanWin", s_DEFAULT_SCAN_WINDOW_MS);
    m_scanDuration      = pref.getULong("scanDur", s_DEFAULT_SCAN_DURATION_SEC);
    m_scanActively      = pref.getBool("scanAct", s_DEFAULT_SCAN_ACTIVE);
    m_scanStartInterval = pref.getULong("scanSInt", s_DEFAULT_SCAN_TIME_MS);
    for(uint8_t i=0; i < MAX_BLE_DEVICES; i++) {
        snprintf(parserKey, 16, "%s%d", parserKeyEnPrefix, i);
        m_deviceParsers[i].enabled = pref.getBool(parserKey, false);
        snprintf(parserKey, 16, "%s%d", parserKeyAddrPrefix, i);
        m_deviceParsers[i].addr[0] = '\0';
        pref.getString(parserKey, m_deviceParsers[i].addr, BLE_ADDR_LEN);
        snprintf(parserKey, 16, "%s%d", parserKeyParserPrefix, i);
        m_deviceParsers[i].parserType = static_cast<BleParserTypes>(pref.getUShort(parserKey, static_cast<uint16_t>(BleParserTypes::none)));
    }
    pref.end();

    m_scanTimer.setInterval(m_scanStartInterval);
}
void BleConnection::save(Preferences &pref) {
    char parserKey[16];
    pref.begin(s_PREF_NAMESPACE, false);
    pref.putUShort("scanInt", m_scanIntervalMs);
    pref.putUShort("scanWin", m_scanWindowMs);
    pref.putULong("scanDur", m_scanDuration);
    pref.putBool("scanAct", m_scanActively);
    pref.putULong("scanSInt", m_scanStartInterval);
    for(uint8_t i=0; i < MAX_BLE_DEVICES; i++) {
        snprintf(parserKey, 16, "%s%d", parserKeyEnPrefix, i);
        pref.putBool(parserKey, m_deviceParsers[i].enabled);
        snprintf(parserKey, 16, "%s%d", parserKeyAddrPrefix, i);
        pref.putString(parserKey, m_deviceParsers[i].addr);
        snprintf(parserKey, 16, "%s%d", parserKeyParserPrefix, i);
        pref.putUShort(parserKey, static_cast<uint16_t>(m_deviceParsers[i].parserType));
    }
    pref.end();
    if( m_log) {
        m_log->print( __FILE__, __LINE__, 1, "Preferences updated" );
    }
}
void BleConnection::addParser(BleParserTypes type, BleParser *parser) {
    bool found = false;
    for(uint8_t i=0; i < MAX_BLE_DEVICES; i++) {
        if(m_parserTypes[i] == type) {
            m_parsers[i] = parser;
            found = true;
            break;
        }
    }
    if(!found && m_nextParser < MAX_BLE_DEVICES) {
        m_parserTypes[m_nextParser] = type;
        m_parsers[m_nextParser] = parser;
        m_nextParser++;
    }
}
BleParser* BleConnection::getParser(BleParserTypes type) {
    if(type == BleParserTypes::none) return nullptr;
    for(uint8_t i=0; i < MAX_BLE_DEVICES; i++) {
        if(m_parserTypes[i] == type) {
            return m_parsers[i];
        }
    }
    return nullptr;
}
void BleConnection::setBleAddress(uint8_t index, const char *addr) {
    if(index >= MAX_BLE_DEVICES) return;
    strncpy(m_deviceParsers[index].addr, addr, BLE_ADDR_LEN);
}
void BleConnection::setBleParser(uint8_t index, BleParserTypes type) {
    if(index >= MAX_BLE_DEVICES) return;
    m_deviceParsers[index].parserType = type;
}
void BleConnection::setBleEnable(uint8_t index, bool enable) {
    if(index >= MAX_BLE_DEVICES) return;
    m_deviceParsers[index].enabled = enable;
}
void BleConnection::logParsers() {
    if(!m_log) return;
    for(uint8_t i=0; i < MAX_BLE_DEVICES; i++) {
        if(m_deviceParsers[i].enabled) {
            m_log->print( __FILE__, __LINE__, 1, i, static_cast<uint32_t>(m_deviceParsers[i].parserType), m_deviceParsers[i].addr, "BleConnection::logParsers: i, type, addr" );
        }
    }
}
void BleConnection::changeState( uint8_t state) {
    if( m_log) {
        m_log->print( __FILE__, __LINE__, 0x100000, m_state, state, "BleConnection::changeState: m_state, state" );
    }
    m_state = state;
}

void BleConnection::slice( void) {

    switch( m_state) {
        case STATE_RESET:
            BLEDevice::init("ble_esp32");
            m_pBleScan = BLEDevice::getScan();
            m_pBleScan->setAdvertisedDeviceCallbacks(this, true);
            changeState( STATE_IDLE);
        break;
        case STATE_IDLE:
            if( m_requestScan) {
                m_requestScan = false;
                if( m_log) {
                    m_log->print( __FILE__, __LINE__, 1, "Start scan request");
                }
                changeState( STATE_START_SCAN);
            } else if(m_scanStartInterval != 0 && m_scanTimer.hasIntervalElapsed()) {
                m_scanTimer.setInterval(m_scanStartInterval);
                if( m_log) {
                    m_log->print( __FILE__, __LINE__, 1, m_scanStartInterval, "Start scan regular: m_scanStartInterval");
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
                    m_log->print( __FILE__, __LINE__, 1, m_results.getCount(), "Scan complete: count");
                }
                m_resultsReceived = false;
                changeState( STATE_IDLE);
            }
        break;
        case STATE_OFF:
            // Wait for reboot or wake up
        break;
        default:
            if( m_log) {
                m_log->print( __FILE__, __LINE__, 1, m_state, "Invalid state: m_state");
            }
            changeState( STATE_RESET);
        break;
    }
}

void BleConnection::onResult(BLEAdvertisedDevice advertisedDevice) {
    BLEAddress address = advertisedDevice.getAddress();
    for(uint8_t i=0; i < MAX_BLE_DEVICES; i++) {
        if(!m_deviceParsers[i].enabled) continue;
        BleParser* parser = getParser(m_deviceParsers[i].parserType);
        BLEAddress addrToParse(m_deviceParsers[i].addr);
        // BAM - 20260206 - Latest BLEAddress == also compares an addr type, which this code is not tracking or interested in
        //  so need do a direct memcmp on the raw bytes.
        //if(address == addrToParse) {
        if(memcmp(address.getNative(), addrToParse.getNative(), ESP_BD_ADDR_LEN) == 0) {
            m_devices[0].payloadLen = 0;
            strcpy(m_devices[0].addr, address.toString().c_str());
            if (advertisedDevice.haveManufacturerData()) {
                int len = advertisedDevice.getManufacturerData().length();
                const char* data = advertisedDevice.getManufacturerData().c_str();
                memcpy(m_devices[0].payload, data, len);
                m_devices[0].payloadLen = (uint8_t) len;
                m_devices[0].valid = true;

                if(parser) {
                    parser->setData(m_devices[0]);
                    parser->parse();
                }
            } else {
                if(m_log) {
                    m_log->print( __FILE__, __LINE__, 1, address.toString().c_str(), "match but no mfg data, address");
                }
            }
        }
    }

    if(m_log && m_bleLogState != BLE_LOG_NONE) {
        m_log->print( __FILE__, __LINE__, 1, (uint32_t) address.getType(), address.toString().c_str(), "BleConnection: type, address");
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

void BleConnection::off() {
    if(m_pBleScan) {
        m_pBleScan->stop();
    }
    changeState( STATE_OFF);
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