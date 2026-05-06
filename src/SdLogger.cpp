#include "SdLogger.h"

#include <SPI.h>
#include <SD.h>
#include <cstring>
#include <cstdlib>

#include "esp_log_custom.h"

static const char* TAG = "SDCard ";

// For SD Card access
SPIClass sd_spi(HSPI);

SdLogger::SdLogger() :
    m_cs(0)
{
    m_timer.setInterval(SD_CONN_CHECK_MS);
}

void SdLogger::begin(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t cs) {
    m_cs = cs;
    sd_spi.begin(sck, miso, mosi, cs);
    if(!SD.begin(cs, sd_spi)) {
        ESP_LOGW(TAG, "SD Card begin failed");
    } else {
        logSdCardStatus();
    }
}

void SdLogger::loop() {
    if(m_timer.isNextInterval()) {
        ESP_LOGI(TAG, "SD Connection check");
        if(SD.cardType() == CARD_NONE) {
            ESP_LOGI(TAG, "SD card not connected, retrying...");
            if(!SD.begin(m_cs, sd_spi)) {
                ESP_LOGW(TAG, "SD card not found");
            } else {
                logSdCardStatus();
            }
        }
    }
}

void SdLogger::logSdCardStatus() {
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE){
        ESP_LOGI(TAG, "No SD card attached");
        return;
    }

    if(cardType == CARD_MMC){
        ESP_LOGI(TAG, "SD Card Type: MMC");
    } else if(cardType == CARD_SD){
        ESP_LOGI(TAG, "SD Card Type: SDSC");
    } else if(cardType == CARD_SDHC){
        ESP_LOGI(TAG, "SD Card Type: SDHC");
    } else {
        ESP_LOGI(TAG, "SD Card Type: UNKNOWN");
    }

    ESP_LOGI(TAG, "SD Card Size: %llu kB", SD.cardSize() / (1024));
    ESP_LOGI(TAG, "Total space: %llu kB", SD.totalBytes() / (1024));
    ESP_LOGI(TAG, "Used space: %llu kB", SD.usedBytes() / (1024));
}

void SdLogger::testSdCard() {
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
      ESP_LOGI(TAG, "No SD card attached");
      return;
    }

    ESP_LOGI(TAG, "SD Card Type: ");
    if(cardType == CARD_MMC){
      ESP_LOGI(TAG, "MMC");
    } else if(cardType == CARD_SD){
      ESP_LOGI(TAG, "SDSC");
    } else if(cardType == CARD_SDHC){
      ESP_LOGI(TAG, "SDHC");
    } else {
      ESP_LOGI(TAG, "UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    ESP_LOGI(TAG, "SD Card Size: %lluMB", cardSize);

    // listDir(SD, "/", 0);
    // createDir(SD, "/mydir");
    // listDir(SD, "/", 0);
    // removeDir(SD, "/mydir");
    // listDir(SD, "/", 2);
    // writeFile(SD, "/hello.txt", "Hello ");
    // appendFile(SD, "/hello.txt", "World!\n");
    // readFile(SD, "/hello.txt");
    // deleteFile(SD, "/foo.txt");
    // renameFile(SD, "/hello.txt", "/foo.txt");
    // readFile(SD, "/foo.txt");
    // testFileIO("/test.txt");
    ESP_LOGI(TAG, "Total space: %lluMB", SD.totalBytes() / (1024 * 1024));
    ESP_LOGI(TAG, "Used space: %lluMB", SD.usedBytes() / (1024 * 1024));
}

void SdLogger::testFileIO(const char * path) {
  File file = SD.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file){
      len = file.size();
      size_t flen = len;
      start = millis();
      while(len){
          size_t toRead = len;
          if(toRead > 512){
              toRead = 512;
          }
          file.read(buf, toRead);
          len -= toRead;
      }
      end = millis() - start;
      ESP_LOGI(TAG, "%u bytes read in %u ms", flen, end);
      file.close();
  } else {
      ESP_LOGI(TAG, "Failed to open file for reading");
  }


  file = SD.open(path, FILE_WRITE);
  if(!file){
      ESP_LOGI(TAG, "Failed to open file for writing");
      return;
  }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++){
      file.write(buf, 512);
  }
  end = millis() - start;
  ESP_LOGI(TAG, "%u bytes written in %u ms", 2048 * 512, end);
  file.close();
}

void SdLogger::log(const char *filePrefix, const char *record, bool createNew) {
    char filename[128];
    File file;

    if(SD.cardType() == CARD_NONE) return;

    long fileNumber = findLargestNumberInFilenames("/", filePrefix);
    if(fileNumber < 0) {
        // Failed to access SD card, unmount it and try again later
        SD.end();
    } else if(fileNumber == 0) {
        fileNumber = 1;
        snprintf(filename, 128, "/%s_%ld.json", filePrefix, fileNumber);
        file = SD.open(filename, FILE_WRITE);
        if(file) {
            ESP_LOGI(TAG, "File not found, created a new file; %s", filename);
        } else {
            ESP_LOGW(TAG, "Failed to create a new file; %s", filename);
        }
    } else {
        snprintf(filename, 128, "/%s_%ld.json", filePrefix, fileNumber);
        file = SD.open(filename, FILE_APPEND);
        if(file) {
            if(createNew || file.size() > SD_FILE_MAX_SIZE) {
                file.close();
                fileNumber += 1;
                snprintf(filename, 128, "/%s_%ld.json", filePrefix, fileNumber);
                file = SD.open(filename, FILE_WRITE);
                if(file) {
                    ESP_LOGI(TAG, "Created a new file; %s", filename);
                } else {
                    ESP_LOGW(TAG, "Failed to create a new file; %s", filename);
                }
            } else {
                ESP_LOGI(TAG, "Opened existing file; %s", filename);
            }
        } else {
            ESP_LOGW(TAG, "Failed to open existing file; %s", filename);
        }
    }

    if(!file) {
        return;
    }

    size_t numWritten = file.write((uint8_t *)record, strlen(record));
    file.close();
    ESP_LOGI(TAG, "Wrote %lu bytes", numWritten);
}

long SdLogger::findLargestNumberInFilenames(const char* dir, const char* prefix) {
    File root = SD.open(dir);
    if (!root || !root.isDirectory()) {
        ESP_LOGI(TAG, "Failed to open directory: %s", dir);
        return -1;
    }

    long maxNum = 0;
    File file = root.openNextFile();
    while (file) {
        const char* name = file.name();
        size_t prefixLen = strlen(prefix);
        if (strncmp(name, prefix, prefixLen) == 0) {
            // Find the first sequence of digits after the prefix
            // +1 to skip the underscore
            const char* numStart = name + prefixLen + 1;
            char* endPtr;
            long num = strtol(numStart, &endPtr, 10);
            if (endPtr != numStart && num > maxNum) {
                maxNum = num;
            }
        }
        file = root.openNextFile();
    }
    root.close();
    return maxNum;
}