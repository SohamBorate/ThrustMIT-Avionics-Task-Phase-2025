#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
#define D4_1  4         //Pin of 1st D4184 module
#define D4_2  33        //Pin of 2nd D4184 module
#define RYLR  Serial2
#define ANAL1 19        //Anal pins are not used
#define ANAL2 18
#define ANAL3 17
#define TPMIN 16        //Pin TP mini weight indicator
#define TRANS 20        //Pin of Honeywell TJE pressure transducer


enum class states : uint8_t {
  safe = 0,
  arm = 1,
  launch = 2,
  failure = 3,
  finished = 4
};                    //States of the rocket motor
states STATE = states::safe;    //Current state of the motor
String received;
states state_received;          //State of the motor received via RYLR
File data;
float thrust;
float pressure;

void setup() {
  // put your setup code here, to run once:
  if(!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD card init failed.");
  };
  data = SD.open("no_hopes_big_L.csv", FILE_WRITE);
  data.println("Thrust,Pressure");      //CSV file headings
  pinMode(D4_1, OUTPUT);
  pinMode(D4_2, OUTPUT);
  RYLR.begin(9600); 
  while (!RYLR) {;}                     //Wait for RYLR to connect
  RYLR.println("AT");
  delay(100);
  RYLR.println("AT+ADDRESS=2");
  delay(100);
  RYLR.println("AT+NETWORKID=10");
  delay(100);
  RYLR.println("AT+BAND=915000000");
  delay(100);
}

void loop() {
  if (RYLR.available()) {
    received = RYLR.readString();
  }
  if(received.indexOf("A") > 0) {state_received = states::arm;}     //If Arm command is given, 'A' will be received
  if ((state_received == states::arm)&&(STATE == states::safe)) {
    STATE = states::arm;
    while(true) {                   //Stops checking for arm command, now only logging and launch command
      received = RYLR.readString();
      if(received.indexOf("L") > 0) {state_received = states::launch;}      //'L' for launch command
      thrust = (float) (analogRead(TPMIN) * 5) / 1023.0;
      pressure = (float) (analogRead(TRANS) * 5) / 1023.0;
      String dat = String(thrust) + "\t" + String(pressure);
      data.println(String(thrust)+","+String(pressure)); 
      RYLR.println("AT+SEND=1,"+String(dat.length())+","+dat);
      if((state_received == states::launch)&&(STATE == states::arm)) {
        digitalWrite(D4_1, HIGH);   //Fire the rocket motor
        digitalWrite(D4_2, HIGH);
        STATE = states::launch;
      }
    }
  }
}
