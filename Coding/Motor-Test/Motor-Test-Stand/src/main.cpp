#include "main.h"
#include "rylr.h"
#include "sd_card.h"

/*
primary mosfet
redundant mosfet
rylr
micro sd card
pressure transducer
load cell
*/

/*
BOOT => waiting for initialisation of all systems
STANDBY => wait for ignition
IGNITE => fire both mosfets
RECOVER => stop
*/

enum MACHINE_STATE {
    BOOT,
    STANDBY,
    IGNITE,
    RECOVER
};

MACHINE_STATE machine_state;

const uint32_t telemetry_delay = 1000 / telemetry_frequency * 1000; // microseconds

value_dtype read_thrust();
value_dtype read_pressure();

void setup() {
    machine_state = BOOT;
    if (!sd_card_setup()) {
        Serial.println("sd card fail");
    }
    if (!rylr_setup()) {
        Serial.println("rylr fail");
        return;
    }

    pinMode(primary_mosfet_pin, OUTPUT);
    pinMode(redundant_mosfet_pin, OUTPUT);
    pinMode(load_cell_pin, INPUT);
    pinMode(pressure_transducer_pin, INPUT);

    // switch from 10-bit to 12-bit adc for higher resolution
    analogReadResolution(12);

    machine_state = STANDBY;

    ////////// STANDBY

    while (machine_state != IGNITE) {
        String data = "STANDBY";
        rylr_send(data);
        data = rylr_receive_parse(rylr_receive());
        if (data == "IGNITE") {
            machine_state = IGNITE;
        };
        delay(100);
    }

    ////////// IGNITE

    digitalWrite(primary_mosfet_pin, HIGH);
    digitalWrite(redundant_mosfet_pin, HIGH);

    // initial 3 second record to turn off mosfet

    uint32_t time_spent = 0;
    uint32_t timeout = 3 * 1000 * 1000; // microseconds
    int curr_thrust = read_thrust();
    int curr_pressure = read_pressure();
    uint32_t curr_micros = micros();
    uint32_t record_start = curr_micros;
    uint32_t last_telemetry_ping = curr_micros;

    while (time_spent <= timeout) {
        curr_micros = micros();
        curr_thrust = read_thrust();
        curr_pressure = read_pressure();
        sd_card_data_log(curr_micros, curr_thrust, curr_pressure);
        time_spent = curr_micros - record_start;
        if (curr_micros - last_telemetry_ping > telemetry_delay) {
            rylr_send((String)curr_micros + "," + (String)curr_thrust + "," + (String)curr_pressure);
            last_telemetry_ping = curr_micros;
        }
    }

    digitalWrite(primary_mosfet_pin, LOW);
    digitalWrite(redundant_mosfet_pin, LOW);

    // final full length record

    time_spent = 0;
    timeout = record_duration * 1000;
    record_start = millis();

    time_spent = 0;
    timeout = record_duration * 1000 * 1000; // microseconds
    curr_thrust = read_thrust();
    curr_pressure = read_pressure();
    curr_micros = micros();
    record_start = curr_micros;
    last_telemetry_ping = curr_micros;

    while (time_spent <= timeout) {
        curr_micros = micros();
        curr_thrust = read_thrust();
        curr_pressure = read_pressure();
        sd_card_data_log(curr_micros, curr_thrust, curr_pressure);
        time_spent = curr_micros - record_start;
        if (curr_micros - last_telemetry_ping > telemetry_delay) {
            rylr_send((String)curr_micros + "," + (String)curr_thrust + "," + (String)curr_pressure);
            last_telemetry_ping = curr_micros;
        }
    }

    machine_state = RECOVER;

    ////////// RECOVER

    rylr_send("RECOVER");
    sd_card_close();
}

void loop() {}

//// use other utility software to convert voltage values to actual units

// does not take into account the mass flux, weight
value_dtype read_thrust() {
    return (value_dtype)analogRead(load_cell_pin);
};

value_dtype read_pressure() {
    return (value_dtype)analogRead(pressure_transducer_pin);
};
