#ifndef SD_CARD_H
#define SD_CARD_H

#include <Arduino.h>
#include <SdFat.h>
#include <RingBuf.h>

// Use Teensy SDIO
#define SD_CONFIG SdioConfig(FIFO_SDIO)

#define SD_CARD_PIN BUILTIN_SDCARD

int sd_card_setup();
// void sd_card_component_log(String component, String type, String remark);
void sd_card_data_log(value_dtype thrust, value_dtype pressure);
void sd_card_close();

#endif
