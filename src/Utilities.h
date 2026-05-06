#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <stdio.h>

void getRtcTimeStr(char *ts, size_t maxLen);

void startCpuPerf(uint32_t durationMs);
void runCpuPerfTask(void *arg);

void printHeapStats(void);
void printChipInfo(void);
void printSdkVersion(void);

#endif // UTILITIES_H_