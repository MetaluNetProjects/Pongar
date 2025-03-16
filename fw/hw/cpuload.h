// CPU load measuring utility
#pragma once

#include "fraise.h"
#include <string>

class CpuLoad {
private:
    absolute_time_t sensor_time;
    absolute_time_t reset_time;
    unsigned int count_us;
    std::string name;
public:
    CpuLoad(std::string n): name(n) {}
    void start() {
        sensor_time = get_absolute_time();
    }
    void stop() {
        count_us += absolute_time_diff_us(sensor_time, get_absolute_time());
    }
    float get_load() {
        float load = (100.0 * count_us) / absolute_time_diff_us(reset_time, get_absolute_time());
        fraise_printf("l load %s %f\n", name.c_str(), load);
        return load;
    }
    void reset() {
        count_us = 0;
        reset_time = get_absolute_time();
    }
};

