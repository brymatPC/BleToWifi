#ifndef VICTRON_DEVICE_H
#define VICTRON_DEVICE_H

#include <stdint.h>
#include <BleParser.h>

class DebugLog;

class VictronDevice : public BleParser {
private:
    uint8_t *m_key;
    DebugLog* m_log;
    bleDeviceData_t m_data;

    void decrypt();
public:
    VictronDevice();
    virtual ~VictronDevice() { }

    void init(DebugLog *log) {m_log = log;}

    void setKey(const char *key);
    virtual void setData(bleDeviceData_t &data) {m_data = data;}
    virtual void parse();
};

#endif // VICTRON_DEVICE_H