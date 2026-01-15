#include "debounce.hpp"

Debounce::Debounce(int pin, unsigned int delay) {
    this->pin = pin;
    this->delay = delay;
    this->reading = this->state = this->lastState = digitalRead(this->pin);
    this->last_time = millis();
    this->wait = false;
}

int Debounce::read() {
    this->reading = digitalRead(this->pin);
    if (this->reading != this->lastState) {
        this->last_time = millis();
        this->wait = true;
    }
    if ((!this->wait)&&((millis() - this->last_time) > this->delay)) {
        if (this->state != this->reading) {
            this->state = this->reading;
        }
    }
    this->lastState = this->reading;
    return this->state;
}
