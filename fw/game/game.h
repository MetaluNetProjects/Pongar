// global game

#pragma once
#include "sound/speaker.h"
#include "sound/sound_command.h"
#include "sound/words.h"
#include "sound/audiolayer.h"
#include "players.h"
#include "gfx/gfx.h"
#include "scorelog.h"

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

class GameMode;
class Game {
private:
    static const int PLAYERS_READY_SECONDS = 3;
    static const int PLAYERS_STABLE_SECONDS = 8;
    static const int NO_PLAYER_SECONDS = 8;
    absolute_time_t update_time = get_absolute_time();
    int game_players_count = 0;
    absolute_time_t players_ready_timeout = get_absolute_time();
    absolute_time_t players_stable_timeout = get_absolute_time();
    absolute_time_t players_say_stable_timeout = get_absolute_time();
    absolute_time_t noplayer_timeout = get_absolute_time();
    absolute_time_t last_game_endtime = get_absolute_time();
    absolute_time_t next_alpague_time = get_absolute_time();
    GameMode *game_mode = nullptr;
    AudioLayer &audio;
    Chaser chaser;
    uint8_t volume = 255;
    bool wait_saying = false;
    int get_ms_since_last_game() {
        return to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(last_game_endtime);
    }
public:
    enum GameState {STOP, PREPARE, WAIT_STABLE, PLAYING, RESTART, STANDBY} state = STOP;
    Game(Scorelog &_log, AudioLayer &_audio): audio(_audio), scorelog(_log) {};
    Speaker speaker;
    Players players;
    Scorelog &scorelog;
    static const int PERIOD_MS = 10;
    void init(int tx_pin);
    void prepare();
    void prepare_restart();
    void start();
    void restart();
    void stop();
    void standby();
    bool update();
    void receivebytes(const char* data, uint8_t len);
    inline int get_players_count() {
        return game_players_count;
    }
    inline void sfx(SoundCommand c, int p1 = 0, int p2 = 0, int p3 = 0) {
        audio.command(c, p1, p2, p3);
    }
    void pixels_update();
    int get_state() {
        return state;
    }
    void set_volume(uint8_t vol);
    uint8_t get_volume() { return volume; }
};

class GameMode {
protected:
    Game &game;
    Speaker &speaker;
public:
    GameMode(Game &_game): game(_game), speaker(game.speaker) {};
    virtual ~GameMode() {};
    virtual int get_max_players() = 0;
    virtual void start() = 0;
    virtual void restart() = 0;
    virtual void update() = 0;
    virtual void pixels_update() {
        game.pixels_update();
    }
};

