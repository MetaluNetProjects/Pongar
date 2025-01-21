// global game

#pragma once
#include "sound.h"
#include "wavplayer.h"
#include "players.h"
#include "sound_command.h"
#include "words.h"

/*class Players {
  private:
    static const int STEADY_MS = 1500;
    int steady_count = 0;
    int pre_steady_count = 0;
    absolute_time_t steady_timeout;
  public:
    void update();
    inline int get_steady_count(){return steady_count;}
    inline int get_count(){return players_count;}
};*/

class GameMode;
class Game {
  private:
    static const int PLAYERS_READY_SECONDS = 4;
    enum {STOP, WAIT_SAYING, PREPARE, PLAYING} mode = STOP;
    absolute_time_t update_time;
    int game_players_count;
    absolute_time_t players_ready_timeout;
    int players_ready_okcount;
    GameMode *game_mode;
    AudioLayer audio;
    WavPlayer wavplayer;
    int say_mode = 1;

    void change_players_count(int count);
  public:
    Players players;
    const int update_ms = 40;
    void init(int audio_pin, int tx_pin);
    void prepare();
    void start();
    void stop();
    void update();
    void receivebytes(const char* data, uint8_t len);
    inline void say(Words w) { wavplayer.play(say_mode, (int)w); }
    inline void saysilence(int ms) { wavplayer.silence(ms); }
    inline void sayclear() { wavplayer.clear(); }
    inline bool is_saying() { return wavplayer.is_playing();}
    inline void sfx(SoundCommand c, int p1 = 0, int p2 = 0, int p3 = 0) {audio.command(c, p1, p2, p3);}
    void pixel_update_players();
};

extern Game game;

class GameMode {
  public:
    virtual ~GameMode() {};
    virtual void init() = 0;
    virtual void update() = 0;
    inline void say(Words w) { game.say(w);}
    inline void saysilence(int ms) {game.saysilence(ms);}
    inline bool is_saying() {return game.is_saying();}
    inline void sayclear() {game.sayclear();}
    inline void sfx(SoundCommand c, int p1 = 0, int p2 = 0, int p3 = 0) {game.sfx(c, p1, p2, p3);}
};


