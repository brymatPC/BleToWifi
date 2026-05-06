#include "Utilities.h"

#include <esp32-hal.h>
#include <esp_chip_info.h>
#include <time.h>

void getRtcTimeStr(char *ts, size_t maxLen) {
    struct tm timeinfo;
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(ts, maxLen, "%FT%H:%M:%SZ", &timeinfo);
}

static bool m_runCpuPerf = false;
static uint32_t m_durationMs = 100;
static const char* TAG = "Perf   ";

#define ARRAY_SIZE_OFFSET   5   //Increase this if print_real_time_stats returns ESP_ERR_INVALID_SIZE
#define MAX_TASKS_TO_TRACK  20
static void print_real_time_stats(TickType_t xTicksToWait) {
    TaskStatus_t start_array[MAX_TASKS_TO_TRACK];
    TaskStatus_t end_array[MAX_TASKS_TO_TRACK];
    UBaseType_t start_array_size, end_array_size;
    configRUN_TIME_COUNTER_TYPE start_run_time, end_run_time;
    esp_err_t ret;

    if(uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET >= MAX_TASKS_TO_TRACK) {
      ESP_LOGI(TAG, "cpu stats: numTasks=%u, array offset=%u", uxTaskGetNumberOfTasks(), (unsigned)ARRAY_SIZE_OFFSET);
      return;
    }

    // Allocate array to store current task states
    start_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    // Get current task states
    start_array_size = uxTaskGetSystemState(start_array, start_array_size, &start_run_time);
    if (start_array_size == 0) {
        ESP_LOGI(TAG, "cpu stats: start array size is 0");
        return;
    }

    vTaskDelay(xTicksToWait);

    // Allocate array to store tasks states post delay
    end_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    // Get post delay task states
    end_array_size = uxTaskGetSystemState(end_array, end_array_size, &end_run_time);
    if (end_array_size == 0) {
        ESP_LOGI(TAG, "cpu stats: end array size is 0");
        return;
    }

    // Calculate total_elapsed_time in units of run time stats clock period.
    uint32_t total_elapsed_time = (end_run_time - start_run_time);
    if (total_elapsed_time == 0) {
        return;
    }

    ESP_LOGI(TAG, "| Task | Run Time | Core | Percentage");
    // Match each task in start_array to those in the end_array
    for (int i = 0; i < start_array_size; i++) {
        int k = -1;
        for (int j = 0; j < end_array_size; j++) {
            if (start_array[i].xHandle == end_array[j].xHandle) {
                k = j;
                //Mark that task have been matched by overwriting their handles
                start_array[i].xHandle = NULL;
                end_array[j].xHandle = NULL;
                break;
            }
        }
        // Check if matching task found
        if (k >= 0) {
            uint32_t task_elapsed_time = end_array[k].ulRunTimeCounter - start_array[i].ulRunTimeCounter;
            uint32_t percentage_time = (task_elapsed_time * 100UL) / (total_elapsed_time * CONFIG_FREERTOS_NUMBER_OF_CORES);
            ESP_LOGI(TAG, "| %s | %" PRIu32" | %d | %" PRIu32"%%", start_array[i].pcTaskName, task_elapsed_time, start_array[i].xCoreID, percentage_time);
        }
    }

    // Print unmatched tasks
    for (int i = 0; i < start_array_size; i++) {
        if (start_array[i].xHandle != NULL) {
            ESP_LOGI(TAG, "| %s | Deleted", start_array[i].pcTaskName);
        }
    }
    for (int i = 0; i < end_array_size; i++) {
        if (end_array[i].xHandle != NULL) {
            ESP_LOGI(TAG, "| %s | Created", end_array[i].pcTaskName);
        }
    }
}

void startCpuPerf(uint32_t durationMs) {
    m_durationMs = durationMs;
    m_runCpuPerf = true;
}

void runCpuPerfTask(void *arg) {
    while(1) {
        if(m_runCpuPerf) {
            print_real_time_stats(pdMS_TO_TICKS(m_durationMs));
            m_runCpuPerf = false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void printHeapStats(void) {
    size_t total = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    size_t free = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    size_t min = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
    size_t largest = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
    ESP_LOGI(TAG, "Heap: total %lu, free %lu, min %lu, largest %lu", total, free, min, largest);
}
void printChipInfo(void) {
    esp_chip_info_t chipInfo;
    esp_chip_info(&chipInfo);
    ESP_LOGI(TAG, "Chip: model=%u revision=%u cores=%u", (unsigned)chipInfo.model, (unsigned)chipInfo.revision, (unsigned)chipInfo.cores);
}
void printSdkVersion(void) {
    ESP_LOGI(TAG, "SDK: version=%s", esp_get_idf_version());
}