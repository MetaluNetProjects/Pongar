// glabal game

#pragma once
#include "sound.h"
#include "players.h"

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

class Game {
  private:
    static const int INIT_PERIOD = 5000;
    static const int MIN_PERIOD = 1200;
    static const int PLAYERS_READY_SECONDS = 4;
    static const int SCORE_MAX = 12;
    enum {STOP, PREPARE, PLAYING} mode = STOP;
    absolute_time_t update_time;
    const int update_ms = 40;
    int period_ms = INIT_PERIOD;
    float pan, tilt;
    float pan_change_amp = 45.0;
    float pan_delta = 0, tilt_delta = 0;
    AudioLayer audio;
    Players players;
    int game_players_count;
    absolute_time_t players_ready_timeout;
    int players_ready_okcount;
    int score;

    void change_players_count(int count);
    //void change_mode(int m);
    void say_score();
    void game_over();
    void win();
  public:
    void init(int audio_pin, int tx_pin);
    void prepare();
    void start();
    void stop();
    void update();
    void set_period_ms(int ms) { period_ms = ms;}
    void receivebytes(const char* data, uint8_t len);
};

extern Game game;
