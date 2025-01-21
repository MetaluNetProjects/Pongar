// Players position

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EXTERN_P
#define EXTERN_P extern
#endif

#define PLAYERS_MAX 20
extern int players_separation; // 30
EXTERN_P int players_count;
EXTERN_P uint16_t players_pos[PLAYERS_MAX];

void players_find(const uint16_t *distance_array);

#ifdef __cplusplus
}
#endif

class Players {
  private:
    static const int STEADY_MS = 1500;
    int steady_count = 0;
    int pre_steady_count = 0;
    absolute_time_t steady_timeout;
  public:
    void update();
    inline int get_steady_count(){return steady_count;}
    inline int get_count(){return players_count;}
};


