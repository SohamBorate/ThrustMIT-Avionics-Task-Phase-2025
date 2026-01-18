#ifndef debounce_h
#define debounce_h

#include "Arduino.h"

class Debounce {
    public:
        Debounce(int, unsigned int);
        int read();

    private:
        int pin, state, lastState, reading;
        unsigned long last_time, delay;
        bool wait;
};
#endif
