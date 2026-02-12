#include "AppManager.h"
#include <utility/DebugLog.h>

#include <Preferences.h>

static char s_resetUnknownStr[]     = "unknown";
static char s_resetPowerOnStr[]     = "power-on";
static char s_resetExtStr[]         = "external";
static char s_resetSoftwareStr[]    = "Software";
static char s_resetPanicStr[]       = "panic";
static char s_resetIntWdogStr[]     = "interrupt watchdog";
static char s_resetTaskWdogStr[]    = "task watchdog";
static char s_resetOthWdogStr[]     = "other watchdog";
static char s_resetDeepSleepStr[]   = "deep sleep";
static char s_resetBrownOutStr[]    = "brown out";
static char s_resetSDIOStr[]        = "sdio";
static char s_resetUsbStr[]         = "usb";
static char s_resetJtagStr[]        = "jtag";
static char s_resetEFuseStr[]       = "efuse";
static char s_resetPowerGlitchStr[] = "power glitch";
static char s_resetCpuLockupStr[]   = "cpu lockup";

const char AppManager::s_PREF_NAMESPACE[] = "sys";
const uint32_t AppManager::s_DEFAULT_RUN_TIME_MS = 0;
const uint32_t AppManager::s_DEFAULT_SLEEP_TIME_MS = 0;
const uint32_t AppManager::s_STATUS_INTERVAL_MS = 30000;

RTC_DATA_ATTR static int m_bootCount = 0;

typedef enum {
  STATE_RESET     = 0,
  STATE_RUNNING   = 1,
  STATE_SLEEP_REQ = 2,
  STATE_SLEEP     = 3,
  STATE_OFF       = 4,

} appStates_t;

AppManager::AppManager(const char* appName, const char* appVersion, DebugLog *dbg) :
    m_appName(appName),
    m_appVersion(appVersion),
    m_log(dbg),
    m_runTimeMs(s_DEFAULT_RUN_TIME_MS),
    m_sleepTimeMs(s_DEFAULT_SLEEP_TIME_MS),
    m_sleepEnabled(true),
    m_state(STATE_RESET)
{
    resetReasonStartup = esp_reset_reason();
    m_bootCount++;

    m_timer.setInterval(5000);
}
AppManager::~AppManager() {

}

void AppManager::init(Preferences &pref) {
    pref.begin(s_PREF_NAMESPACE, true);
    m_runTimeMs = pref.getULong("runt", s_DEFAULT_RUN_TIME_MS);
    m_sleepTimeMs = pref.getULong("slpt", s_DEFAULT_SLEEP_TIME_MS);
    pref.end();
}
void AppManager::save(Preferences &pref) {
    pref.begin(s_PREF_NAMESPACE, false);
    pref.putULong("runt", m_runTimeMs);
    pref.putULong("slpt", m_sleepTimeMs);
    pref.end();
    if( m_log) {
        m_log->print( __FILE__, __LINE__, 1, "AppManager::save: pref updated" );
    }
}

void AppManager::slice( void) {
    if(m_timer.hasIntervalElapsed()) {
        m_timer.setInterval(s_STATUS_INTERVAL_MS);
        if(m_log) {
            const char *resetStr = resetReasonToString(resetReasonStartup);
            m_log->print(__FILE__, __LINE__, 1, "System Status Only, not a reboot!");
            m_log->print(__FILE__, __LINE__, 1, m_appName, "s_appName");
            m_log->print(__FILE__, __LINE__, 1, m_appVersion, "s_appVersion");
            m_log->print(__FILE__, __LINE__, 1, m_bootCount, "bootCount");
            m_log->print(__FILE__, __LINE__, 1, resetReasonStartup, resetStr, "Reset Reason - resetReasonStartup, resetReasonStartupStr");
            m_log->printX(__FILE__, __LINE__, 1, 0xA5, "Hex test");
        }
    }

    switch(m_state) {
        case STATE_RESET:
            m_state = STATE_RUNNING;
        break;
        case STATE_RUNNING:
            if(m_sleepEnabled && m_runTimeMs > 0 && m_sleepTimeMs > 0 && millis() > m_runTimeMs) {
                if(m_log) {
                    m_log->print(__FILE__, __LINE__, 1, m_sleepTimeMs, "Sleep enabled - m_sleepTimeMs");
                }
                if(preSleep_cb) {
                    preSleep_cb();
                }
                m_state = STATE_SLEEP_REQ;
            }
        break;
        case STATE_SLEEP_REQ:
            // Force sleep after 3 seconds
            if(millis() > m_runTimeMs + 3000) {
                m_state = STATE_SLEEP;
            } else if(sleepReady_cb == nullptr || sleepReady_cb()) {
                m_state = STATE_SLEEP;
            }
        break;
        case STATE_SLEEP:
            // No exit from this state
            esp_deep_sleep(1000LL * m_sleepTimeMs);
        break;
        default:
            m_state = STATE_RESET;
        break;
    }
}

const char *AppManager::resetReasonToString(esp_reset_reason_t reason) {
    char *result;
    switch (reason) {
    case ESP_RST_POWERON:
        result = s_resetPowerOnStr;
        break;
    case ESP_RST_EXT:
        result = s_resetExtStr;
        break;
    case ESP_RST_SW:
        result = s_resetSoftwareStr;
        break;
    case ESP_RST_PANIC:
        result = s_resetPanicStr;
        break;
    case ESP_RST_INT_WDT:
        result = s_resetIntWdogStr;
        break;
    case ESP_RST_TASK_WDT:
        result = s_resetTaskWdogStr;
        break;
    case ESP_RST_WDT:
        result = s_resetOthWdogStr;
        break;
    case ESP_RST_DEEPSLEEP:
        result = s_resetDeepSleepStr;
        break;
    case ESP_RST_BROWNOUT:
        result = s_resetBrownOutStr;
        break;
    case ESP_RST_SDIO:
        result = s_resetSDIOStr;
        break;
    case ESP_RST_USB:
        result = s_resetUsbStr;
        break;
    case ESP_RST_JTAG:
        result = s_resetJtagStr;
        break;
    case ESP_RST_EFUSE:
        result = s_resetEFuseStr;
        break;
    case ESP_RST_PWR_GLITCH:
        result = s_resetPowerGlitchStr;
        break;
    case ESP_RST_CPU_LOCKUP:
        result = s_resetCpuLockupStr;
        break;
    case ESP_RST_UNKNOWN:
    default:
        result = s_resetUnknownStr;
        break;
    }
    return result;
}