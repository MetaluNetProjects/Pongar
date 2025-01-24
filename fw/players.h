// Players position

#pragma once
#include <set>
#include "fraise.h"

class Player {
  private:
    uint16_t position = 0;
    bool _exists = false;
    absolute_time_t creation_time;
    absolute_time_t update_time;
  public:
    static const int TIMEOUT_MS = 4000;
    void create(uint16_t pos) {
        _exists = true;
        position = pos;
        creation_time = update_time = get_absolute_time();
    }
    void destroy() {
        _exists = false;
    }
    inline bool exists() { return _exists;}
    inline uint16_t get_position() { return position;}
    inline void update(uint16_t pos) { position = pos; update_time = get_absolute_time(); }
    inline bool older_than_ms(absolute_time_t t, int ms) {
        return (to_ms_since_boot(t) + ms) < to_ms_since_boot(get_absolute_time());
    }
    inline bool too_old() {
        return older_than_ms(update_time, TIMEOUT_MS);
    }
    inline bool visible() { return _exists && older_than_ms(creation_time, 500) && !older_than_ms(update_time, 500); }
};

class Players {
  public:
    static const int PLAYERS_MAX = 20;
  private:
    static const int STEADY_MS = 1500;
    int steady_count = 0;
    int pre_steady_count = 0;
    absolute_time_t steady_timeout;
    uint16_t players_raw_pos[PLAYERS_MAX];
    int players_raw_count = 0;
    int players_separation = 45;
    Player players[PLAYERS_MAX];
    std::set<int> players_set;
    void follow_players();
    void cleanup();
    int create_player(uint16_t pos);
  public:
    void find_players(const uint16_t *distance_array);
    void set_raw_pos(uint16_t *pos, int count); // for debugging from Pd
    void update();
    inline int get_steady_count(){return steady_count;}
    inline int get_count(){return players_set.size();}
    int16_t get_pos(int player); // return -1 if not visible
    const std::set<int> &get_set() {return players_set;}
    bool is_visible(int player) {return players[player].visible();}
};


