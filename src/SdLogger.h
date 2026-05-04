#ifndef SD_LOGGER_H_
#define SD_LOGGER_H_

#include <stdint.h>

#define SD_FILE_MAX_SIZE (1024 * 1024)

class SdLogger {
public:
    SdLogger();

    void begin();
    void testSdCard();

    void log(const char *filePrefix, const char *record);

private:

    long findLargestNumberInFilenames(const char* dir, const char* prefix);
    void testFileIO(const char * path);
};

#endif // SD_LOGGER_H_