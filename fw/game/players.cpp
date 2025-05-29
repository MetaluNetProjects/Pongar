// Players position

#include "fraise.h"
#define EXTERN_P
#include "players.h"
#include "config.h"
#include <math.h>
#include <vector>

Position null_position{
    .angle = 0,
    .distance = 12000,
    .size = 0
};

template <unsigned int N>
unsigned int MostFrequent<N>::next(unsigned int value) {
    std::array<unsigned int, 8> frequencies{0};
    if(value > frequencies.size() - 1) value = frequencies.size() - 1;
    last_values[index] = value;
    index = (index + 1) % N;
    unsigned int most_frequent_value = 0;
    unsigned int most_frequent_freq = 0;
    for(auto v: last_values) {
        frequencies[v]++;
        if(frequencies[v] > most_frequent_freq) {
            most_frequent_freq = frequencies[v];
            most_frequent_value = v;
        }
    }
    return most_frequent_value;
}

#define angle_distance(a, b) abs(((a) - (b) + 180 + 720) % 360 - 180)

void Players::find_players_light(const uint16_t *distance_array) {
    lidar_tab = distance_array;
    raw_count = 0;
    int start = 0;
    int hole_width = 0;
    // start after a wide enough hole
    //Position p = {.angle = 0, .distance = (unsigned uint16_t)config.distance_max, .size = (unsigned uint16_t)players_separation_mm};
    int players_separation = 5;//p.get_size_deg();
    for(start = 0; start < 360; start++) {
        if(distance_array[start] < config.distance_max) hole_width = 0;
        else hole_width++;
        if(hole_width > players_separation) break;
    }
    if(start == 359) return; // no hole!
    bool detected = false;
    int player_width = 0;
    int player_start = 0;
#define INDEX ((start + index) % 360)
    for(int index = 0; index < 360; index++) {
        bool near_enough = distance_array[INDEX] < config.distance_max;
        if(!near_enough) hole_width++;
        else hole_width = 0;
        if(detected) {
            if(hole_width >= players_separation) {
                // create new raw
                if(raw_count >= PLAYERS_MAX) break;
                if(player_width > 5) {
                    uint16_t distance = config.distance_max;
                    for(int i = 0; i < player_width; i++) {
                        int d = distance_array[(player_start + i) % 360];
                        if(distance > d) distance = d;
                    }
                    raw_positions[raw_count] = {
                        .angle = (uint16_t)((player_start + player_width / 2 + config.lidar_angle_offset) % 360),
                        .distance = distance,
                        .size = 200
                    };
                    //printf("new raw pos angle %d width %d\n", raw_positions[raw_count].angle, player_width);
                    raw_count++;
                }
                detected = false;
            }
            else player_width++;
        } else {
            if(near_enough) {
                player_start = INDEX;
                player_width = 1;
                detected = true;
            }
        }
    }
    //follow_players();
    //printf("raw count= %d\n", raw_count);
    too_close = false;
    std::set<int> positions;
    for(int i = 0; i < raw_count; i++) {
        int a = raw_positions[i].angle;
        bool found = false;
        for(int p : positions) {
            if(angle_distance(a, p) < 50) {
                found = true;
                break;
            }
        }
        if(!found) positions.insert(a);
        if(raw_positions[i].distance < config.game_distance_min) too_close = true;
    }

    players_count = most_frequent_count.next(positions.size());
    //printf("count %d most frequent %d\n", (int)positions.size(), players_count);
}

void Players::update() {
    if(steady_count == players_count) {
        pre_steady_count = -1;
        return;
    }
    if(pre_steady_count != players_count) {
        pre_steady_count = players_count;
        steady_timeout = make_timeout_time_ms(STEADY_MS);
        return;
    }
    if(time_reached(steady_timeout)) {
        steady_count = pre_steady_count;
    }
}

bool Players::get_too_close() {
    return (!config.disable_too_close) && too_close;
}

bool Players::presence_at(int angle, int tolerance) {
    for(int i = 0; i < raw_count; i++) {
        if(abs((raw_positions[i].angle - angle + 180 + 720) % 360 - 180) < tolerance) return true;
    }
    return false;
}
