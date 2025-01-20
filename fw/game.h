// global game

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

class GameMode {
  public:
    virtual ~GameMode() {};
    virtual void init() = 0;
    virtual void update() = 0;
};

class Game {
  private:
    static const int PLAYERS_READY_SECONDS = 4;
    enum {STOP, PREPARE, PLAYING} mode = STOP;
    absolute_time_t update_time;
    int game_players_count;
    absolute_time_t players_ready_timeout;
    int players_ready_okcount;
    GameMode *game_mode;

    void change_players_count(int count);
  public:
    const int update_ms = 40;
    AudioLayer audio;
    Players players;
    void init(int audio_pin, int tx_pin);
    void prepare();
    void start();
    void stop();
    void update();
    void receivebytes(const char* data, uint8_t len);
};

extern Game game;
