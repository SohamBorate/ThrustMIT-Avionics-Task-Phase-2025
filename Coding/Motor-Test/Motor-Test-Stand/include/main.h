#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <stdint.h>

#define primary_mosfet_pin 2
#define redundant_mosfet_pin 3
#define pressure_transducer_pin 4
#define load_cell_pin 5

#define window_size 10

// store data as which data type
#define value_dtype uint8_t

// packed ensures no padding bytes (unnecessary) (compiler is the imposter)
struct __attribute__((packed)) data_entry {
    uint32_t timestamp; // microseconds
    value_dtype thrust;
    value_dtype pressure;
};

/*
values should be set based on a rough idea of what
the thrust curve is gonna look like based on
openmotor simulations
*/
#define fire_start_threshold 4
#define fire_stop_threshold 4
#define detect_start_timeout 10 // seconds
#define detect_stop_timeout 20
#define extra_timeout 4
// maximum detect_start_timeout + detect_stop_timeout + extra_timeout seconds of data logging

#endif
