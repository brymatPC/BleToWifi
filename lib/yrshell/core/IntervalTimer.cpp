
#include "HardwareSpecific.h"
#include "IntervalTimer.h"
#include "HiResTimer.h"

IntervalTimer::IntervalTimer( unsigned intervalInMilliSeconds) {
    setInterval( intervalInMilliSeconds);
}
bool IntervalTimer::isNextInterval(void) {
    bool rc = hasIntervalElapsed();
    if( rc) {
        m_start += m_interval;
    }
    return rc;
}
void IntervalTimer::setInterval( unsigned intervalInMilliSeconds) {
    m_start = HiResTimer::getMillis();
    m_interval = intervalInMilliSeconds;
}
bool IntervalTimer::hasIntervalElapsed( void) {
    return (HiResTimer::getMillis() - m_start) >= m_interval;
}
