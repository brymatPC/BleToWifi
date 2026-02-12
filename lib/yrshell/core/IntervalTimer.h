#ifndef IntervalTimer_h
#define IntervalTimer_h

#include "HardwareSpecific.h"

class IntervalTimer {
private:
   unsigned m_start, m_interval;
public:
    IntervalTimer( unsigned intervalInMilliSeconds = 0);
    void setInterval( unsigned intervalInMilliSeconds);
    bool hasIntervalElapsed( void);
    bool isNextInterval(void);
};

#endif

