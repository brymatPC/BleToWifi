#include "Utilities.h"

#include <time.h>

void getRtcTimeStr(char *ts, size_t maxLen) {
    struct tm timeinfo;
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(ts, maxLen, "%FT%H:%M:%SZ", &timeinfo);
}