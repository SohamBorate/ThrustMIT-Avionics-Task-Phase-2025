#include <Arduino.h>
#include <stdint.h>

const int pot_in = A0;
const int btn_pin = 12;
uint32_t ignite_timestamp;

void setup() {
    Serial.begin(115200);
    pinMode(pot_in, INPUT);
    pinMode(btn_pin, INPUT);
    Serial.println("ARMED");
    while (digitalRead(btn_pin) != 1) {delay(1);}
    Serial.println("FIRE");
    ignite_timestamp = micros();
}

void loop() {
    uint32_t timestamp = micros() - ignite_timestamp;
    int adc = analogRead(pot_in);
    Serial.print(timestamp);
    Serial.print(",");
    Serial.println(adc);
}
