#include "SdLogger.h"

#include <SPI.h>
#include <SD.h>

#include "esp_log_custom.h"

static const char* TAG = "SDCard ";


SdLogger::SdLogger() {

}

void SdLogger::begin() {

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