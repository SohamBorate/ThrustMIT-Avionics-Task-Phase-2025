#include "main.h"
#include <cmath>

int main() {
    std::ofstream data_csv("data.bin", std::ios::binary);
    
    if (!data_csv) {
        std::cout << "could not open data.bin" << std::endl;
        return 1;
    }

    data_entry entry;
    for (int i = 0; i < 100; i++) {
        entry.timestamp = i * 10000;
        entry.thrust = i * sin(i) + 1000;
        entry.pressure = i * cos(i) + 1000;
        data_csv.write(reinterpret_cast<char*>(&entry), sizeof(entry));
    }

    data_csv.close();
    return 0;
}
