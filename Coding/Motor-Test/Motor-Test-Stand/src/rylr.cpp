#include <Arduino.h>

#include "main.h"
#include "rylr.h"
#include "sd_card.h"

const String rylr_password = RYLR_PASSWORD;

int rylr_setup() {
    Serial.begin(BAUD_RATE);

    RYLR.begin(115200);
    RYLR.println("AT"); // each command must end with \r\n, but println automatically adds it at the end

    while (!RYLR.available()) {}
    String response = RYLR.readStringUntil('\n');
    response.trim(); // reponse might contain \r in the end
    if (response == "+OK") {
        Serial.println("rylr998 up and running");
        // sd_card_component_log("rylr998", "initiate", "all okay");
        return 1;
    } else {
        Serial.println("rylr998 could not initiate");
        // sd_card_component_log("rylr998", "error", response);
        return 0;
    }

    RYLR.println("AT+CPIN=" + rylr_password);
}

void rylr_send(String data) {
    int length = data.length();
    String command = "AT+SEND=0," + String(length) + "," + data;
    RYLR.println(command);

    // sd_card_component_log("rylr998", "send", command);

    while (!RYLR.available()) {}
    String response = RYLR.readStringUntil('\n');
    response.trim();

    if (response == "+OK") {
        Serial.println("rylr998 data transmission successful");
    } else {
        Serial.print("Unexpected RYLR output: "); Serial.println(response);
        // sd_card_component_log("rylr998", "error", response);
        return;
    }
}

String rylr_receive() {
    String data;
    while (!RYLR.available()) {}
    data = RYLR.readStringUntil('\n');
    data.trim();

    if (!data.startsWith("+RCV=")) {
        Serial.print("Unexpected RYLR output: "); Serial.println(data);
        // sd_card_component_log("rylr998", "error", data);
        return "";
    }

    // sd_card_component_log("rylr998", "receive", data);

    return data;
}

String rylr_receive_parse(String data) {
    // String split[5];
    int p1 = data.indexOf(',');
    int p2 = data.indexOf(',', p1 + 1);
    int p3 = data.indexOf(',', p2 + 1);
    // int p4 = data.indexOf(',', p3 + 1);
    // split[0] = data.substring(0, p1).substring(5);
    // split[1] = data.substring(p1 + 1, p2);
    // split[2] = data.substring(p2 + 1, p3);
    // split[3] = data.substring(p3 + 1, p4);
    // split[4] = data.substring(p4 + 1);
    // Serial.print("Transmitter address: "); Serial.println(split[0]);
    // Serial.print("Data length: "); Serial.println(split[1]);
    // Serial.print("Data: "); Serial.println(split[2]);
    // Serial.print("RSSI: "); Serial.println(split[3]);
    // Serial.print("SNR: "); Serial.println(split[4]);
    return Serial.println(data.substring(p2 + 1, p3));
}
