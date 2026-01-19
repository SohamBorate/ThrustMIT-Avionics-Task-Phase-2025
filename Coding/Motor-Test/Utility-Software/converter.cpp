/*
reads data.bin and converts it into a csv
data.bin only has raw adc values to have faster data logging
this program will apply formulas to convert raw voltage
to actual units, and then save that to a csv
*/

#include "main.h"
#include "moving_avg.h"

// WARNING: not taking into account the weight of the motor, mass flux, etc
double convert_thrust (value_dtype adc) {
    return ((double)adc / (double)adc_max) * (double)thrust_range_newtons;
}

double convert_pressure (value_dtype adc) {
    return ((double)adc / (double)adc_max) * (double)pressure_range_pascals;
}

int main() {
    std::ofstream data_csv("data.csv");
    std::ifstream data_bin("data.bin", std::ios::binary);

    if (!data_bin) {
        std::cout << "could not open data.bin" << std::endl;
    }

    if (!data_csv) {
        std::cout << "could not open data.csv" << std::endl;
    }

    data_csv << "timestamp,thrust_adc,thrust_newtons,thrust_newtons_smooth,pressure_adc,pressure_pascals,pressure_pascals_smooth" << "\n";

    data_entry entry;
    double thrust_newtons = 0.0;
    double pressure_pascals = 0.0;
    Moving_Average<double, moving_avg_window_size> thrust_newtons_smooth;
    Moving_Average<double, moving_avg_window_size> pressure_pascals_smooth;

    while (data_bin.read(reinterpret_cast<char*>(&entry), sizeof(data_entry))) {
        thrust_newtons = convert_thrust(entry.thrust);
        pressure_pascals = convert_pressure(entry.pressure);
        thrust_newtons_smooth.append(thrust_newtons);
        pressure_pascals_smooth.append(pressure_pascals);
        std::cout
            << std::to_string((double)entry.timestamp / 1000000.0) // usec to sec
            << "," << std::to_string(entry.thrust)
            << "," << std::to_string(thrust_newtons)
            << "," << std::to_string(thrust_newtons_smooth.curr_avg)
            << "," << std::to_string(entry.pressure)
            << "," << std::to_string(pressure_pascals)
            << "," << std::to_string(pressure_pascals_smooth.curr_avg)
            << "\n";
        data_csv
            << std::to_string((double)entry.timestamp / 1000000.0)
            << "," << std::to_string(entry.thrust)
            << "," << std::to_string(thrust_newtons)
            << "," << std::to_string(thrust_newtons_smooth.curr_avg)
            << "," << std::to_string(entry.pressure)
            << "," << std::to_string(pressure_pascals)
            << "," << std::to_string(pressure_pascals_smooth.curr_avg)
            << "\n";
    }

    if (data_bin.eof()) {
        std::cout << "reached end of data.bin." << std::endl;
    } else if (data_bin.fail()) {
        std::cout << "error" << std::endl;
    }

    data_bin.close();
    data_csv.close();
    return 0;
}
