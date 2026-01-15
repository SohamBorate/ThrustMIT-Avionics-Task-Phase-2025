#include <Arduino.h>
#include "debounce.hpp"
#define RYLR Serial2          //RYLR Serial object
#define SD_CS BUILTIN_SDCARD  //Chip select pin of the builtin SD card
#define armbutton 1           //For giving Arm and Launch commands
#define launchbutton 2

enum class states : uint8_t {
  safe = 0,
  armed = 1,
  launch = 2,
  failure = 3
};                          //States of the rocket motor
states STATE = states::safe;
Debounce arm = Debounce(armbutton, 100);    //Debouncing has been implemented, check debounce.hpp and debounce.cpp
Debounce launch = Debounce(launchbutton, 100);
String rylrread;            //Data/telemetry read from RYLR
float thrust, pressure;
// put function declarations here:

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  RYLR.begin(9600);
  while(!RYLR) {;} //Wait for RYLR to connect
  RYLR.println("AT");
  delay(100);
  RYLR.println("AT+ADDRESS=1");     //Device address, must not be same as receiver
  delay(100);
  RYLR.println("AT+NETWORKID=10");  //Network address, must be same as receiver
  delay(100);
  RYLR.println("AT+BAND=915000000");
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  if((STATE == states::safe)&&(arm.read())) {
    STATE = states::armed;
    Serial.println("The device is ARMED.");
    RYLR.println("AT+SEND=2,1,A");
    Serial.println("Thrust, pressure");
    while(true) {
      if((STATE == states::armed)&&(launch.read())) {
        STATE = states::launch;
        Serial.println("The device has been LAUNCHED.");
        RYLR.println("AT+SEND=2,1,L");
        while(true) {
          rylrread = RYLR.readString();
          Serial.println(rylrread);
          if (arm.read() && launch.read()) {
            Serial.println("Motor test has ended. The device will now stop receiving data.");
            break;
          }
        }
        break;
      }
    }
  }
}
