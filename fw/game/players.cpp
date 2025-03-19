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
        bool near = distance_array[INDEX] < config.distance_max;
        if(!near) hole_width++;
        else hole_width = 0;
        if(detected) {
            if(hole_width >= players_separation) {
                // create new raw
                if(raw_count >= PLAYERS_MAX) break;
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
                raw_count++;
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
    players_count = positions.size();

    /*for(int i = 0; i < PLAYERS_MAX; i++) {
        if(i < raw_count) {
            if(players[i].exists()) {
                //printf("update player %d\n", i);
                players[i].update(raw_positions[i]);
            }
            else {
                //printf("create player %d\n", i);
                players[i].create(raw_positions[i]);
                players_set.insert(i);
            }
        } else {
            players[i].destroy();
            players_set.erase(i);
        }
    }*/
}

#define testloops(i,s) do{i++; if(i > 1000) { printf("l too many loops in \"%s\"\n", s); return;}}while(0)
void Players::find_players(const uint16_t *distance_array) {
    lidar_tab = distance_array;
    raw_count = 0;
    int start_angle = -1;
    int current_angle = 0;
    int loops = 0;
    while(current_angle != start_angle) {
        testloops(loops, "main find_players");
        //printf("start=%d current:%d\n", start_angle, current_angle);
        int dist = distance_array[current_angle];
        int separation_deg_half = deg_from_size(raw_separation_mm, dist) / 2;
        if(dist < config.distance_max) {
            // find left border
            //int da = -1;
            int left = (current_angle + 359) % 360;
            int last_good = current_angle;
            int hole = 0;
            int loops2 = 0;
            while(/*da >= -separation_deg_half &&*/ left != current_angle) {
                testloops(loops2, "find left");
                if(abs(distance_array[left] - dist) < raw_separation_mm) {
                    last_good = left;
                    hole = 0;
                } else {
                    hole++;
                    if(hole >= separation_deg_half) break;
                }
                //da--;
                left = (left + 359) % 360;
            }
            if(left == current_angle) left = (current_angle + 1) % 360;
            if(start_angle == -1) start_angle = left;
            left = last_good;

            // find right border
            //da = 1;
            int right = (current_angle + 1) % 360;
            last_good = current_angle;
            hole = 0;
            loops2 = 0;
            while(/*da <= separation_deg_half && */ right != start_angle) {
                //da++;
                testloops(loops2, "find right");
                if(abs(distance_array[right] - dist) < raw_separation_mm) {
                    last_good = right;
                    hole = 0;
                } else {
                    hole++;
                    if(hole >= separation_deg_half) break;
                }
                right = (right + 1) % 360;
            }
            current_angle = right;
            right = last_good;
            int size_deg = abs((right - left + 180 + 360) % 360 - 180);
            if(size_deg > 0) {
                int middle_angle = (left + size_deg / 2) % 360;
                raw_positions[raw_count++] = {
                    .angle = (uint16_t)((middle_angle + config.lidar_angle_offset) % 360),
                    .distance = (uint16_t)dist,
                    .size = (uint16_t)size_from_deg(size_deg, dist)
                };
                //printf("l add raw %d %d %d %d\n", middle_angle, dist, size_deg, size_from_deg(size_deg, dist));
                //printf("start:%d current:%d left:%d right:%d\n", start_angle, current_angle, left, right);
                if(raw_count == PLAYERS_MAX) {
                    printf("l mx number of raw positions reached\n");
                    break;
                }
            }
            if(current_angle == start_angle) break;
        }
        current_angle = (current_angle + 1) % 360;
        if(start_angle == -1) start_angle = 0;
    }
    follow_objects();
}

int Players::create_object(Position &pos) {
    if(objects_set.size() == PLAYERS_MAX) return -1; // too many objects
    int i;
    for(i = 0; i < PLAYERS_MAX; i++) if(!objects[i].exists()) break;
    if(i == PLAYERS_MAX) return -1;
    objects[i].create(pos);
    objects_set.insert(i);
    return i;
}

int Players::create_player(Position &pos) {
    if(players_set.size() == PLAYERS_MAX) return -1; // too many players
    int i;
    for(i = 0; i < PLAYERS_MAX; i++) if(!players[i].exists()) break;
    if(i == PLAYERS_MAX) return -1;
    players[i].create(pos);
    players_set.insert(i);
    return i;
}

// erase objects and players that haven't been seen since too long
void Players::cleanup() {
    for (auto it = objects_set.begin(); it != objects_set.end();) {
        int p = *it;
        if(objects[p].too_old()) {
            objects[p].destroy();
            it = objects_set.erase(it);
        }
        else ++it;
    }
    for (auto it = players_set.begin(); it != players_set.end();) {
        int p = *it;
        if(players[p].too_old()) {
            players[p].destroy();
            it = players_set.erase(it);
        }
        else ++it;
    }
}

class TmpObject : public Position {
public:
    int num;
    int select = -1;
    uint16_t dist[Players::PLAYERS_MAX];
    TmpObject(int n, Position &p): Position(p), num(n) {}
};

void Players::follow_objects() {
    std::vector<TmpObject> temp_objects;
    std::vector<TmpObject> found_tobjs;
    // fill temp_objects from raw positions
    for(int i = 0; i < raw_count; i++) temp_objects.emplace_back(i, raw_positions[i]);
    // compute distances from all temp_objects to all current objects
    for(int i = 0; i < (int)temp_objects.size(); i++)
        for(uint j : objects_set)
            temp_objects[i].dist[j] = temp_objects[i].distance_to(objects[j]);

    std::set<int> objects_to_test = objects_set;

    while(!temp_objects.empty() && !objects_to_test.empty()) {
        // find the closest {temp_object, object} pair
        int min_dist = 1000;
        auto tp = temp_objects.begin();
        int p = 0;
        for(auto tpit = temp_objects.begin(); tpit != temp_objects.end(); tpit++)
            for(auto j : objects_to_test)
                if(tpit->dist[j] < min_dist) {
                    min_dist = tpit->dist[j];
                    tp = tpit;
                    p = j;
                }
        // remove the temp_object and the object from the waiting lists
        found_tobjs.emplace_back(*tp);
        found_tobjs.back().select = p;
        temp_objects.erase(tp);
        objects_to_test.erase(p);
    }

    /*for(int p: objects_to_test) {
        if(objects[p].exists()) printf("l couldn't find obj for object %d\n", p);
    }*/

    for(auto tp : found_tobjs) {
        //printf("raw %d links to object %d\n", tp.num, tp.select);
        // update the object position and timeout
        objects[tp.select].update(tp);
    }

    // create new objects for remaining temp_objects
    bool too_many = false;
    for(auto tpit = temp_objects.begin(); tpit != temp_objects.end() && !too_many; tpit++) {
        create_object(*tpit);
    }

    // temp copy objs to players
    /*players_set = objects_set;
    for(int i = 0; i < PLAYERS_MAX; i++) {
        if(objects[i].exists()) {
            if(players[i].exists()) players[i].update(objects[i]);
            else players[i].create(objects[i]);
        }
        else players[i].destroy();
    }*/
    follow_players();
    cleanup();
}

void Players::associate(int element, int player) {
    //printf("associate element %d to player %d\n", element, player);
    if(objects[element].exists() && players[player].exists()) {
        objects[element].associate(player);
        players[player].add_element(element);
        players[player].update(players[player]);
    }
}

void Players::disassociate(int element, int player) {
    //printf("disassociate element %d from player %d\n", element, player);
    objects[element].associate(-1);
    players[player].remove_element(element);
}

void Players::follow_players() {
    for(int element : objects_set) {
        int player = objects[element].get_associate();
        if(player == -1 || !players[player].exists()) { // accociate to the closest player, or create a new one
            int min_dist = 12000;
            player = -1;
            for(int p : players_set) {
                int d = objects[element].distance_to(players[p]);
                if(d < players_separation_mm && d < min_dist) {
                    min_dist = d;
                    player = p;
                }
            }
            if(player == -1 && objects[element].visible()) player = create_player(objects[element]);
            if(player != -1) associate(element, player);
        } else { // check the associate isn't too far
            int d = objects[element].distance_to(players[player]);
            if(d > players_separation_mm) disassociate(element, player);
            else associate(element, player);
        }
    }
    // update players position
    for(int p : players_set) {
        Player &player = players[p];
        auto &elts = player.get_elements();
        if(elts.size()) {
            int angle = 0;
            int distance = 0;
            int angle0 = -1;
            for(int el : elts) {
                int a = objects[el].angle;
                if(angle0 == -1) angle0 = a;
                else {
                    if(a < angle0 - 180) a += 360;
                    else if(a > angle0 + 180) a -= 360;
                }
                //printf("sumangle+=%d ", a);
                //if(abs(a - 360) < abs(a)) a += 360;
                angle += a;
                distance += objects[el].distance;
                //printf("player %d: include element %d\n", p, el);
            }
            player.angle = (angle / elts.size() + 360) % 360;
            //printf("sumangle=%d player.angle=%d\n", angle, player.angle);
            /*if(abs((player.angle - a + 180 + 360 * 4) % 360 - 180) > (abs((player.angle - a + 360 * 4) % 360 - 180))) {
                player.angle = (player.angle + 180) % 360;
            }*/
            player.distance = distance / elts.size();
        }
    }
}

void Players::set_raw_pos(Position *pos, int count) {
    raw_count = MIN(count, PLAYERS_MAX);
    for(int i = 0; i < raw_count; i++) raw_positions[i] = pos[i];
    follow_objects();
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

Position& Players::get_object_pos(int el) {
    if(el >= 0 && el < PLAYERS_MAX && objects[el].exists()) return objects[el];
    return null_position;
}

Position& Players::get_pos(int player) {
    if(player >= 0 && player < PLAYERS_MAX && players[player].exists()) return players[player];
    return null_position;
}

bool Players::presence_at(int angle, int tolerance) {
    for(int i = 0; i < raw_count; i++) {
        if(abs((raw_positions[i].angle - angle + 180 + 720) % 360 - 180) < tolerance) return true;
    }
    return false;
}
