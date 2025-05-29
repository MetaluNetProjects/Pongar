// Players position

#pragma once
#include <set>
#include <cmath>
#include <array>
#include "fraise.h"

inline uint16_t deg_from_size(uint16_t size, uint16_t distance) {
    return size * 180.0 / (distance * 3.14159);
}
inline uint16_t size_from_deg(uint16_t angle, uint16_t distance) {
    return (angle * distance * 3.14159) / 180.0;
}
inline int modulo(int a, int b) {
    const int result = a % b;
    return result >= 0 ? result : result + b;
}

struct Position {
    uint16_t angle;     // degree
    uint16_t distance;  // mm
    uint16_t size;      // mm
    inline void set_size_deg(int deg) {
        size = size_from_deg(deg, distance);
    }
    inline int get_size_deg() {
        return deg_from_size(size, distance);
    }
    inline void set(Position& p) {
        angle = p.angle;
        distance = p.distance;
        size = p.size;
    }
    uint16_t distance_to(Position &p) {
        int d = sqrt(distance * distance + p.distance * p.distance - 2 * distance * p.distance * cos((angle - p.angle) * 3.1459 / 180.0));
        int sum_rad = (size + p.size) / 2;
        if(d > sum_rad) d -= sum_rad;
        else d = 0;
        return d;
    }
};

template <unsigned int N>
class MostFrequent {
    std::array<unsigned int, N> last_values{0};
    unsigned int index = 0;
    public:
        unsigned int next(unsigned int value); // get next value and return the most frequent one among Nth last ones
};

class Players {
public:
    static const int PLAYERS_MAX = 20;
private:
    static const int STEADY_MS = 1500;
    int players_count = 0;
    int steady_count = 0;
    int pre_steady_count = 0;
    absolute_time_t steady_timeout = at_the_end_of_time;
    Position raw_positions[PLAYERS_MAX];
    int raw_count = 0;
    int raw_separation_mm = 150;
    const uint16_t *lidar_tab = 0;
    bool too_close = false;

    MostFrequent<10> most_frequent_count;
public:
    void find_players_light(const uint16_t *distance_array);
    void update();
    bool count_is_steady() {
        return time_reached(steady_timeout);
    }
    inline int get_steady_count() {
        return steady_count;
    }
    bool presence_at(int angle, int tolerance);
    bool get_too_close();
};


