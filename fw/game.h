// global game

#pragma once
#include "sound.h"
#include "wavplayer.h"
#include "players.h"
#include "sound_command.h"
#include "words.h"
#include "gfx/gfx.h"

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

class GameMode;
class Game {
  private:
    static const int PLAYERS_READY_SECONDS = 4;
    enum {STOP, WAIT_SAYING, PREPARE, PLAYING, STANDBY} mode = STOP;
    absolute_time_t update_time;
    int game_players_count;
    absolute_time_t players_ready_timeout;
    int players_ready_okcount;
    GameMode *game_mode;
    AudioLayer audio;
    WavPlayer wavplayer;
    Chaser chaser;
    int say_mode = 1;
    float dim;
    void change_players_count(int count);
  public:
    Players players;
    static const int PERIOD_MS = 40;
    void init(int audio_pin, int tx_pin);
    void prepare();
    void start();
    void stop();
    void standby();
    void update();
    void receivebytes(const char* data, uint8_t len);
    inline int get_players_count() { return game_players_count; }
    inline void say(Words w) { wavplayer.play(say_mode, (int)w); }
    inline void saysilence(int ms) { wavplayer.silence(ms); }
    inline void sayclear() { wavplayer.clear(); }
    inline bool is_saying() { return wavplayer.is_playing(); }
    inline void sfx(SoundCommand c, int p1 = 0, int p2 = 0, int p3 = 0) { audio.command(c, p1, p2, p3); }
    void pixels_update();
};

extern Game game;

class GameMode {
  public:
    virtual ~GameMode() {};
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void pixels_update() { game.pixels_update(); }
    inline void say(Words w) { game.say(w);}
    inline void saysilence(int ms) { game.saysilence(ms); }
    inline bool is_saying() { return game.is_saying(); }
    inline void sayclear() { game.sayclear(); }
    inline void sfx(SoundCommand c, int p1 = 0, int p2 = 0, int p3 = 0) { game.sfx(c, p1, p2, p3); }
};


