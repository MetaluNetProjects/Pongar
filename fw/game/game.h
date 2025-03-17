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
    enum {STOP, PREPARE, WAIT_STABLE, PLAYING, RESTART, STANDBY} mode = STOP;
    absolute_time_t update_time;
    int game_players_count;
    absolute_time_t players_ready_timeout;
    absolute_time_t players_stable_timeout;
    absolute_time_t players_say_stable_timeout;
    absolute_time_t noplayer_timeout;
    absolute_time_t last_game_endtime;
    absolute_time_t next_alpague_time;
    GameMode *game_mode;
    AudioLayer &audio;
    Chaser chaser;
    int say_mode = 1;
    bool wait_saying = false;
    int get_ms_since_last_game() {
        return to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(last_game_endtime);
    }
public:
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

