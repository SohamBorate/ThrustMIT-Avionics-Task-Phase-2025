#include "main.h"
#include "moving_avg.h"
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
FAILURE => something went wrong
*/

enum MACHINE_STATE {
    BOOT,
    STANDBY,
    IGNITE,
    FAILURE,
    RECOVER
};

MACHINE_STATE machine_state;

value_dtype read_thrust();
value_dtype read_pressure();

void setup() {
    machine_state = BOOT;
    if (!sd_card_setup()) {
        // sd_card_component_log("machine", "state change", "failure");
        Serial.println("sd card fail");
    }
    if (!rylr_setup()) {
        // sd_card_component_log("machine", "state change", "failure");
        Serial.println("rylr fail");
        return;
    }

    /*
    LED for camera evidence
    - ON => STANDBY
    - BLINKING => ARMED
    - ON again => IGNITION
    - OFF => BOOT/FAILURE
    */
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(primary_mosfet_pin, OUTPUT);
    pinMode(redundant_mosfet_pin, OUTPUT);
    pinMode(load_cell_pin, INPUT);
    pinMode(pressure_transducer_pin, INPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    // switch from 10-bit to 12-bit adc for higher resolution
    analogReadResolution(12);

    machine_state = STANDBY;
    // sd_card_component_log("machine", "state change", "standby");

    ////////// STANDBY

    while (machine_state != IGNITE) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        String data = "STANDBY";
        rylr_send(data);
        data = rylr_receive_parse(rylr_receive());
        if (data == "IGNITE") {
            machine_state = IGNITE;
            // sd_card_component_log("machine", "state change", "ignite");
        };
        delay(100);
    }

    ////////// IGNITE
    // maximum 34 seconds of data logging
    // less if checks work as intended

    digitalWrite(LED_BUILTIN, HIGH);

    Moving_Average<value_dtype, window_size> thrust_moving_avg;
    // Moving_Average<value_dtype, window_size> pressure_moving_avg;

    // compare buffer to compare two consecutive values
    // quickly by switching index of the more recent value
    // instead of moving the values themselves
    value_dtype cmp_bff[2];
    cmp_bff[0] = 0;
    cmp_bff[1] = 0;
    int current = 0;
    int old = 1;

    int time_spent = 0;
    int timeout = detect_start_timeout * 1000;
    int start = millis();

    digitalWrite(primary_mosfet_pin, HIGH);
    digitalWrite(redundant_mosfet_pin, HIGH);

    //// detect start

    // comparisons can possibly lead to false FAILURE
    // if data reading rate is high enough
    // high chances burn stage detection is going to fail
    // timeout is the savior

    // less conditions to keep less checks for higher priority to data logging
    while ((cmp_bff[current] - cmp_bff[old]) < (value_dtype)fire_start_threshold && cmp_bff[old] <= cmp_bff[current] && time_spent < timeout) {
        time_spent = millis() - start;
        old = !old;
        current = !current;

        value_dtype value = read_thrust();
        cmp_bff[current] = thrust_moving_avg.append(value);
        sd_card_data_log(value, read_pressure());

        // log only the raw data
        // raw data can be tested with different filters later on
        // sd_card_thrust_log(thrust_moving_avg.append(cmp_bff[current]));
    }

    if (time_spent >= timeout) {
        // failure
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(primary_mosfet_pin, LOW);
        digitalWrite(redundant_mosfet_pin, LOW);
        rylr_send("FAILURE");
        // sd_card_component_log("machine", "state change", "failure");
        // keep moving forward anyways
        // sd_card_close();
        // return;
    }

    //// detect stop

    time_spent = 0;
    timeout = detect_stop_timeout * 1000;
    start = millis();

    while ((cmp_bff[old] - cmp_bff[current]) < (value_dtype)fire_stop_threshold && cmp_bff[old] >= cmp_bff[current] && time_spent < timeout) {
        time_spent = millis() - start;
        old = !old;
        current = !current;

        value_dtype value = read_thrust();
        cmp_bff[current] = thrust_moving_avg.append(value);
        sd_card_data_log(value, read_pressure());

        // log only the raw data
        // raw data can be tested with different filters later on
        // sd_card_thrust_log(thrust_moving_avg.append(cmp_bff[current]));
    }

    digitalWrite(primary_mosfet_pin, LOW);
    digitalWrite(redundant_mosfet_pin, LOW);

    //// log for 4 more seconds

    time_spent = 0;
    timeout = extra_timeout * 1000;
    start = millis();
    while (time_spent < timeout) {
        sd_card_data_log(read_thrust(), read_pressure());
        time_spent = millis() - start;
        // log only the raw data
        // raw data can be tested with different filters later on
        // sd_card_thrust_log(thrust_moving_avg.append(cmp_bff[current]));
    }

    machine_state = RECOVER;
    // sd_card_component_log("machine", "state change", "standby");

    ////////// RECOVER

    digitalWrite(LED_BUILTIN, LOW);
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
