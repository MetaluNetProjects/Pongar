// Players position

#include "fraise.h"
#define EXTERN_P
#include "players.h"
#include "config.h"
#include <math.h>
#include <vector>

void Players::find_players(const uint16_t *distance_array) {
    players_raw_count = 0;
    int start = 0;
    int hole_width = 0;
    // start after a wide enough hole
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
        bool near = distance_array[INDEX] < config.distance_max;
        if(!near) hole_width++;
        else hole_width = 0;
        if(detected) {
            if(hole_width >= players_separation) {
                // create new raw player
                if(players_raw_count >= PLAYERS_MAX) return;
                players_raw_count++;
                players_raw_pos[players_raw_count - 1] = (player_start + player_width / 2 + config.lidar_angle_offset) % 360;
                detected = false;
            }
            else player_width++;
        } else {
            if(near) {
                player_start = INDEX;
                player_width = 1;
                detected = true;
            }
        }
    }
    follow_players();
}

int Players::create_player(uint16_t pos) {
    if(players_set.size() == PLAYERS_MAX) return -1; // too many players
    int i;
    for(i = 0; i < PLAYERS_MAX; i++) if(!players[i].exists()) break;
    players[i].create(pos);
    players_set.insert(i);
    return i;
}

  // erase players that haven't been seen since too long
void Players::cleanup() {
    for (auto it = players_set.begin(); it != players_set.end();) {
        int p = *it;
        if(players[p].too_old()) {
            players[p].destroy();
            it = players_set.erase(it);
        }
        else ++it;
    }
}

struct TmpPlayer {
    int num;
    uint16_t pos;
    uint16_t dist[Players::PLAYERS_MAX];
    TmpPlayer(int n, uint16_t p): num(n), pos(p) {}
};

void Players::follow_players() {
    std::vector<TmpPlayer> tplayers;
      // fill temp_players from raw positions
    for(int i = 0; i < players_raw_count; i++) tplayers.emplace_back(i, players_raw_pos[i]);
      // compute distances from all temp_players to all current players
    for(int i = 0; i < (int)tplayers.size(); i++)
        for(uint j: players_set)
            tplayers[i].dist[j] = abs(tplayers[i].pos - players[j].get_position());

    std::set<int> players_to_test = players_set;

    while(!tplayers.empty() && !players_to_test.empty()) {
          // find the closest {temp_player, player} pair
        int min_dist = 1000;
        auto tp = tplayers.begin();
        int p = 0;
        for(auto tpit = tplayers.begin(); tpit != tplayers.end(); tpit++)
            for(auto j : players_to_test)
                if(tpit->dist[j] < min_dist) {
                    min_dist = tpit->dist[j];
                    tp = tpit;
                    p = j;
                }
          // update the player position and timeout
        players[p].update(tp->pos);
          // remove the temp_player and the player from the waiting lists
        tplayers.erase(tp);
        players_to_test.erase(p);
    }

    // create new players for remaining temp_players
    bool too_many = false;
    for(auto tpit = tplayers.begin(); tpit != tplayers.end() && !too_many; tpit++) {
        create_player(tpit->pos);
    }
    cleanup();
}

void Players::set_raw_pos(uint16_t *pos, int count){
    players_raw_count = MIN(count, PLAYERS_MAX);
    for(int i = 0; i < players_raw_count; i++) players_raw_pos[i] = pos[i];
    follow_players();
}

void Players::update() {
    if(steady_count == players_raw_count) {
        steady_timeout = at_the_end_of_time;
        pre_steady_count = -1;
        return;
    }
    if(pre_steady_count != players_raw_count) {
        pre_steady_count = players_raw_count;
        steady_timeout = make_timeout_time_ms(STEADY_MS);
        return;
    }
    if(time_reached(steady_timeout)) {
        steady_count = pre_steady_count;
        steady_timeout = at_the_end_of_time;
    }
}

int16_t Players::get_pos(int player) {
    int16_t pos = 0;
    if(player < 0 || player >= PLAYERS_MAX || !players[player].exists()) pos = -1;
    else pos = players[player].get_position();
    return pos;
}

