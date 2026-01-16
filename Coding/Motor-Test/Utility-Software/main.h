#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string>

#define moving_avg_window_size 10

// store data as which data type
#define value_dtype uint8_t

// packed ensures no padding bytes (unnecessary) (compiler is the imposter)
// different form of packed attribute
#pragma pack(push,1)
struct data_entry {
    uint32_t timestamp; // microseconds
    value_dtype thrust;
    value_dtype pressure;
};
#pragma pack(pop)

/*
assuming

MCU ANALOG INPUT PIN MAX: 3.3 V (voltage divider will be used)
MCU 12 BIT ADC: 0-4095
*/
#define adc_max 4095

/*
assuming

TE Connectivity FC2311-0000-1000-L LOAD CELL
Range: 453.59 kg (1000 lbf)
Voltage Output: 10 V dc
*/
#define thrust_range_newtons 453.59 * 9.81 // SI units

/*
assuming

Honeywell TJE Pressure Transducer
RANGE CODE: DT (7500 psi)
VOLTAGE OUTPUT OPTION: 2t (0-10 V dc)
*/

#define pressure_range_pascals 7500 * 6894.76 // SI units
