
#include <vector>
#include <iostream>
#include "fraise.h"
#include "config.h"
#include "players.h"

using namespace std;

/* -------------------------- pico sdk compat --------------------------*/
absolute_time_t make_timeout_time_ms(int ms) {
    struct timeval tv;
    struct timeval tv_offset { .tv_sec = ms / 1000, .tv_usec = (ms % 1000) * 1000};
    gettimeofday(&tv, 0);
    timeradd(&tv, &tv_offset, &tv);
    return tv;
}

bool time_reached(absolute_time_t t) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return timercmp(&t, &tv, <);
}

absolute_time_t at_the_end_of_time { .tv_sec = (long)1e6, .tv_usec = 0};

uint8_t fraise_get_uint8() { return 0;}

PongarConfig config;

//-----------------------------------------------------------------------

struct Test {
    int i;
    Test(int ii): i(ii) {}
};

int main() {
    /*vector<Test> v;
    
    v.emplace_back(0);
    Test* back1 = &(v.back());
    v.emplace_back(1);
    Test* back2 = &(v.back());
    cout << "back1:" << back1 << " back2:" << back2 << " v[0]:" << &v[0] << " v[1]: "<< &v[1] << endl;*/
    
    Players p;
    uint16_t pos[] = {50, 150};
    p.set_raw_pos(pos, 2);
    p.get_pos(0);
    p.get_pos(1);
    return 0;
}
