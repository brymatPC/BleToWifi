#ifndef SD_LOGGER_H_
#define SD_LOGGER_H_

#include <stdint.h>
#include <core/IntervalTimer.h>

#define SD_CONN_CHECK_MS (30000)
#define SD_FILE_MAX_SIZE (1024 * 1024)

class SdLogger {
public:
    SdLogger();

    void begin(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t cs);
    void loop();

    void log(const char *filePrefix, const char *record, bool createNew = false);

private:
    uint8_t m_cs;
    IntervalTimer m_timer;
    long findLargestNumberInFilenames(const char* dir, const char* prefix);
    void testFileIO(const char * path);
    void testSdCard();
    void logSdCardStatus();
};

#endif // SD_LOGGER_H_