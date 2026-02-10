#ifndef APP_MANAGER_H_
#define APP_MANAGER_H_

#include <utility/IntervalTimer.h>
#include <utility/Sliceable.h>

class DebugLog;
class Preferences;

typedef void (*preSleepNotificationCallback)(void);
typedef bool (*sleepReadyCallback)(void);

class AppManager : public Sliceable {
private:
    static const char s_PREF_NAMESPACE[];
    static const uint32_t s_DEFAULT_RUN_TIME_MS;
    static const uint32_t s_DEFAULT_SLEEP_TIME_MS;
    static const uint32_t s_STATUS_INTERVAL_MS;

    const char* m_appName;
    const char* m_appVersion;
    DebugLog *m_log;
    esp_reset_reason_t resetReasonStartup;
    IntervalTimer m_timer;

    uint32_t m_runTimeMs;
    uint32_t m_sleepTimeMs;
    bool m_sleepEnabled;

    preSleepNotificationCallback preSleep_cb = nullptr;
    sleepReadyCallback sleepReady_cb = nullptr;

    uint32_t m_state;

    const char *resetReasonToString(esp_reset_reason_t reason);
public:
    AppManager(const char* appName, const char* appVersion, DebugLog *dbg);
    virtual ~AppManager();
    virtual const char* sliceName( void) { return "AppManager"; }
    void init(Preferences &pref);
    void save(Preferences &pref);
    virtual void slice( void);

    void setPreSleepCallback(preSleepNotificationCallback cb) {preSleep_cb = cb; }
    void setSleepReadyCallback(sleepReadyCallback cb) {sleepReady_cb = cb; }

    void setRunTimeMs(uint32_t runTime) {m_runTimeMs = runTime;}
    void setSleepTimeMs(uint32_t sleepTime) {m_sleepTimeMs = sleepTime;}
    void setSleepEnabled(bool enable) {m_sleepEnabled = enable;}

};

#endif //  APP_MANAGER_H_
