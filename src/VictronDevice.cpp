#include "VictronDevice.h"

#include <utility/DebugLog.h>
#include <utility/String.h>

#include <aes/esp_aes.h>

#define VICTRON_KEY_LEN (16)

// be
// 1e
// db
// a7
// 71
// 64
// 0f
// ae
// c9
// 1d
// 68
// 3c
// 70
// ef
// e5
// f1
// F1 E5 EF 70 3C 68 1D C9 AE 0F 64 71 A7 DB 1E BE
//static uint8_t s_DEFAULT_KEY[] = {0xF1, 0xE5, 0xEF, 0x70, 0x3C, 0x68, 0x1D, 0xC9, 0xAE, 0x0F, 0x64, 0x71, 0xA7, 0xDB, 0x1E, 0xBE};
static uint8_t s_DEFAULT_KEY[] = {0xBE, 0x1E, 0xDB, 0xA7, 0x71, 0x64, 0x0F, 0xAE, 0xC9, 0x1D, 0x68, 0x3C, 0x70, 0xEF, 0xE5, 0xF1};

VictronDevice::VictronDevice() {
    m_key = s_DEFAULT_KEY;
    m_data = bleDeviceData_t{};
}
void VictronDevice::setKey(const char *key) {
    uint8_t tempKey[VICTRON_KEY_LEN];
    uint8_t num = 0;
    bool keyValid = true;
    for(uint8_t i=0; i < VICTRON_KEY_LEN*2; i+=2) {
        if(key[i] == '\0' || key[i+1] == '\0') {
            keyValid = false;
            break;
        } else {
            tempKey[num] = charToHex(key[i]) << 4;
            tempKey[num] |= charToHex(key[i+1]);
            num++;
        }
    }
    if(keyValid) {
        memcpy(m_key, tempKey, VICTRON_KEY_LEN);
        m_log->printX( __FILE__, __LINE__, 1, m_key[0], m_key[1], "VictronDevice: key updated: key_0, key_1");
    } else {
        m_log->print( __FILE__, __LINE__, 1, num, "VictronDevice: key invalid, ignoring: num");
    }
}
void VictronDevice::parse() {
    if(m_data.payloadLen == 0 || m_data.payload == nullptr) return;

    if(m_data.payloadLen > 7) {
        uint16_t companyId = (m_data.payload[0] << 8) | (m_data.payload[1]);
        uint8_t dataRecordType = m_data.payload[2];
        uint16_t modelId = (m_data.payload[3] << 8) | (m_data.payload[4]);
        uint8_t readOutType = m_data.payload[5];
        uint8_t recordType = m_data.payload[6];

        if(m_log) {
            m_log->printX( __FILE__, __LINE__, 1, companyId, modelId, "VictronDevice: companyId, modelId");
            m_log->printX( __FILE__, __LINE__, 1, dataRecordType, readOutType, recordType, "VictronDevice: dataRecordType, readOutType, recordType");
        }
    }

    if(m_data.payloadLen >= 25) {
        uint16_t nonce = (m_data.payload[7] << 8) | (m_data.payload[8]);
        uint8_t keyStart = m_data.payload[9];

        // payload 10-25 are the encrypted bytes
        decrypt();

    } else {
        if(m_log) {
            m_log->print( __FILE__, __LINE__, 1, m_data.payloadLen, "VictronDevice - insufficient bytes to parse: payloadLen");
        }
    }
}
void VictronDevice::decrypt() {
    size_t nonce_offset = 0;
    uint8_t nonce_counter[32] = {0};
    uint8_t stream_block[32] = {0};
    uint8_t outputData[32];
    size_t dataLen = m_data.payloadLen - 10;
    esp_aes_context ctx;
    esp_aes_init(&ctx);
    int status = esp_aes_setkey(&ctx, m_key, 128);
    if(status != 0) {
        if(m_log) {
            m_log->print( __FILE__, __LINE__, 1, status, "VictronDevice - failed to start aes: status");
        }
        return;
    }
    // construct the 16-byte nonce counter array by piecing it together byte-by-byte.
    nonce_counter[0] = m_data.payload[7];
    nonce_counter[1] = m_data.payload[8];
    
    status = esp_aes_crypt_ctr(&ctx, dataLen, &nonce_offset, nonce_counter, stream_block, &m_data.payload[10], outputData);
    esp_aes_free(&ctx);

    if (status != 0) {
        if(m_log) {
            m_log->print( __FILE__, __LINE__, 1, status, "VictronDevice - failed to decrypt: status");
        }
    } else {
        // Bits 15:0 - TTG (minutes)
        // Bits 31:16 - Battery voltage (0.01 V)
        // Bits 47:32 - Alarm Reason
        // Bits 63:48 - Aux voltage (0.01 V)
        // Bits 65:64 - Aux Input type
        // Bits 87:66 - Battery Current (0.001A)
        // Bits 107:88 - Consumed Ah (0.1 Ah)
        // Bits 117:108 - State of Charge (0.1%)
        uint16_t timeToGo = (outputData[1] << 8) | (outputData[0]);
        uint16_t batteryVoltage = (outputData[3] << 8) | (outputData[2]);
        // No alarms
        uint16_t auxVoltage = (outputData[7] << 8) | (outputData[6]);
        uint8_t auxType = (outputData[8] & 0x03);
        uint32_t batteryCurrent_u = (outputData[10] << 14) | (outputData[9] << 6) | ((outputData[8] & 0xFC) >> 2);
        uint32_t consumed = ((outputData[13] & 0x0F) << 16) | (outputData[12] << 8) | (outputData[11]);
        uint32_t stateOfCharge = ((outputData[14] & 0x3F) << 4) | ((outputData[13] & 0xF0) >> 4);
        int32_t batteryCurrent = 0;

        if(batteryCurrent_u <= 0x001FFFFF) {
            batteryCurrent = (int32_t) batteryCurrent_u;
        } else {
            batteryCurrent = (int32_t) (0x3FFFFF - batteryCurrent_u + 1);
            // TODO: Add this in once logging of signed values is fixed
            //batteryCurrent *= -1;
        }

        if(m_log) {
            m_log->print( __FILE__, __LINE__, 1, batteryVoltage, batteryCurrent_u, batteryCurrent, "VictronDevice: batteryVoltage, batteryCurrent_u, batteryCurrent");
            //m_log->print( __FILE__, __LINE__, 1, auxVoltage, auxType, "VictronDevice: auxVoltage, auxType");
            //m_log->print( __FILE__, __LINE__, 1, timeToGo, consumed, stateOfCharge, "VictronDevice: timeToGo, consumed, stateOfCharge");
        }

        #ifdef LOG_OUTPUT_DATA
            if(m_log) {
                char outStr[128];
                outStr[0] = '0';
                outStr[1] = 'x';
                for(int i=0; i < 15; i++) {
                    sprintf(&outStr[2+i*2], "%02X", outputData[i]);
                }
                m_log->print( __FILE__, __LINE__, 1, outStr, "VictronDevice: outputData");
            }
        #endif
    }
}