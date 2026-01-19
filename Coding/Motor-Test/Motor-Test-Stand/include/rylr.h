#ifndef RYLR_H
#define RYLR_H

#include <Arduino.h>

#define BAUD_RATE 115200
#define RYLR Serial2 // RX Pin 7, TX Pin 8
#define RYLR_ADDRESS  "0" // 0 stand 1 ground

// flamy end up pointy end down
#define RYLR_PASSWORD "feuped"

int rylr_setup();
void rylr_send(String data);
String rylr_receive();
String rylr_receive_parse(String data);

#endif
