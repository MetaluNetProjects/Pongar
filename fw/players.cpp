// Players position

#include "fraise.h"
#include "lidar.h"
#define EXTERN_P
#include "players.h"
#include "string.h"
#include "config.h"

int players_separation = 30;

void players_find(const uint16_t *distance_array){
    players_count = 0;
    int start = 0;
    int hole_width = 0;
    // start after a wide enough hole
    for(start = 0; start < 360; start++) {
        if(distance_array[start] < config.distance_max) hole_width = 0;
        else hole_width++;
        if(hole_width > players_separation) break;
    }
    if(start == 359) return; // no hole!
    //int index;
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
                // create new player
                if(players_count >= PLAYERS_MAX) return;
                players_count++;
                players_pos[players_count - 1] = (player_start + player_width / 2 + config.lidar_angle_offset) % 360;
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
}


