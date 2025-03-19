// Players position

#pragma once
#include <set>
#include "fraise.h"
#include <math.h>

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

extern Position null_position;

class Object : public Position {
private:
    bool _exists = false;
    absolute_time_t creation_time;
    absolute_time_t update_time;
public:
    static const int TIMEOUT_MS = 8000;
    void create(Position &pos) {
        _exists = true;
        set(pos);
        creation_time = update_time = get_absolute_time();
    }
    void destroy() {
        _exists = false;
    }
    inline bool exists() {
        return _exists;
    }
    inline void update(Position &pos) {
        set(pos);
        update_time = get_absolute_time();
    }
    inline bool older_than_ms(absolute_time_t t, int ms) {
        return (to_ms_since_boot(t) + ms) < to_ms_since_boot(get_absolute_time());
    }
    inline bool too_old() {
        return older_than_ms(update_time, TIMEOUT_MS);
    }
    inline bool visible() {
        return _exists && older_than_ms(creation_time, 500) && !older_than_ms(update_time, 500);
    }
};

class Element : public Object {
    int associated_player = -1;
    absolute_time_t association_time;
public:
    inline void associate(int player) {
        associated_player = player;
        if(player != -1) association_time = get_absolute_time();
    }
    inline int get_associate() {
        return associated_player;
    }
};

class Player : public Object {
    std::set<int> elements;
public:
    void add_element(int element) {
        elements.insert(element);
    }
    void remove_element(int element) {
        elements.erase(element);
    }
    const std::set<int>& get_elements() {
        return elements;
    }
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

    Element objects[PLAYERS_MAX];
    std::set<int> objects_set;
    int create_object(Position &pos);

    int players_separation_mm = 500;
    Player players[PLAYERS_MAX];
    std::set<int> players_set;
    int create_player(Position &pos);

    void associate(int element, int player);
    void disassociate(int element, int player);

    void follow_objects();
    void follow_players();
    void cleanup();
public:
    void find_players_light(const uint16_t *distance_array);
    void find_players(const uint16_t *distance_array);
    void set_raw_pos(Position *pos, int count); // for debugging from Pd
    void update();
    bool count_is_steady() {
        return time_reached(steady_timeout);
    }
    inline int get_steady_count() {
        return steady_count;
    }
    inline int get_count() {
        return players_set.size();
    }

    Position& get_object_pos(int el); // return null_position if not visible
    const std::set<int> &get_object_set() {
        return objects_set;
    }
    bool is_object_visible(int el) {
        return objects[el].visible();
    }

    Position& get_pos(int player); // return null_position if not visible
    const std::set<int> &get_set() {
        return players_set;
    }
    bool is_visible(int player) {
        return players[player].visible();
    }
    bool presence_at(int angle, int tolerance);
    bool get_too_close() {
        return too_close;
    }
};


