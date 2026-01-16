#include "main.h"
#include "sd_card.h"

// make ram available again
#define buffer_size 256 * 1024 // 256 KB
#define pre_alloc_size 100 * 1024 * 1024 // 100 MB, arbitrary number big enough for data

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

    // if (!SD.exists("component_log.csv")) {
    //     component_file = SD.open("component_log.csv", FILE_WRITE);
    //     if (component_file) {
    //         component_file.println("timestamp(ms),component,type,remark");
    //         component_file.close();
    //     }
    // }

    // if (!SD.exists("data")) {
    //     data_file = SD.open("data", FILE_WRITE);
    // }

    // component_file = SD.open("component_log.csv", FILE_WRITE);
    // data_file = SD.open("data", FILE_WRITE);
    // Serial.println("sd card ready");
    // sd_card_component_log("sd card", "status", "ready");

    return 1;
}

// void sd_card_component_log(String component, String type, String remark) {
//     component_file.print(millis());
//     component_file.print(",");
//     component_file.print(component);
//     component_file.print(",");
//     component_file.print(type);
//     component_file.print(",");
//     component_file.println(remark);
//     component_file.flush();
// }

void sd_card_data_log(value_dtype thrust, value_dtype pressure) {
    // buffer[count].timestamp = micros();
    // buffer[count].thrust = thrust;
    // buffer[count].pressure = pressure;
    // count++;
    // if (count >= buffer_size) {
    //     // batch write to binary
    //     // 1 make function treat the buffer as constant bytes that wont change
    //     // 2 send it
    //     data_file.write((const uint8_t*)buffer, sizeof(buffer));
    //     count = 0;
    //     sd_card_flush();
    // }

    data_entry data;
    data.timestamp = micros();
    data.thrust = thrust;
    data.pressure = pressure;
    ring_buffer.write((const uint8_t*)&data, sizeof(data));

    // ring_buffer.print(micros());
    // ring_buffer.print(',');
    // ring_buffer.print(thrust);
    // ring_buffer.print(',');
    // ring_buffer.println(pressure);
    // if (ring_buffer.bytesUsed() >= 512 && !data_file.isBusy()) {
    //     if (ring_buffer.writeOut(512) != 512) {
    //         Serial.println("SD write failed");
    //     }
    // }
}

void sd_card_close() {
    ring_buffer.sync(); // send all remaining
    data_file.truncate(); // pat down the file size
    data_file.flush(); // send sector
    data_file.close();
    SDF.end();
}
