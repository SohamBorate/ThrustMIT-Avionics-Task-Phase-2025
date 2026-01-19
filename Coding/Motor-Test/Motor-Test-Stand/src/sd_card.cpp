#include "main.h"
#include "sd_card.h"

// make ram available again
#define buffer_size 256 * 1024 // 256 KB
#define pre_alloc_size 300 * 1024 * 1024 // 300 MB, arbitrary number big enough for data

#define data_file_name "data.bin"

// data_entry buffer[buffer_size];
// int count = 0;

SdFs SDF;
// FsFile component_file;
FsFile data_file;

RingBuf<FsFile, buffer_size> ring_buffer;

int sd_card_setup() {
    if (!SDF.begin(SD_CONFIG)) {
        Serial.println("sd card fail");
        return 0;
    }

    if (!data_file.open(data_file_name, O_RDWR | O_CREAT | O_TRUNC)) {
        Serial.println("open failed");
        return 0;
    }

    if (!data_file.preAllocate(pre_alloc_size)) {
        Serial.println("preAllocate failed");
        data_file.close();
        return 0;
    }

    // initialize the RingBuf.
    ring_buffer.begin(&data_file);
    ring_buffer.println("timestamp(microseconds),thrust(voltage),pressure(voltage)");

    return 1;
}

void sd_card_data_log(uint32_t timestamp, value_dtype thrust, value_dtype pressure) {
    data_entry data;
    data.timestamp = timestamp;
    data.thrust = thrust;
    data.pressure = pressure;
    ring_buffer.write((const uint8_t*)&data, sizeof(data));
}

void sd_card_close() {
    ring_buffer.sync(); // send all remaining
    data_file.truncate(); // pat down the file size
    data_file.flush(); // send sector
    data_file.close();
    SDF.end();
}
