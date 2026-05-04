#ifndef SD_LOGGER_H_
#define SD_LOGGER_H_

#include <stdint.h>

class SdLogger {
public:
    SdLogger();

    void begin();
    void testSdCard();

private:

    void testFileIO(const char * path);
};

#endif // SD_LOGGER_H_