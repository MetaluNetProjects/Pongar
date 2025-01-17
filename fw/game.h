// glabal game

#pragma once
#include "sound.h"
class Game {
  private:
    enum {STOP, PLAYING} mode = STOP;
    absolute_time_t update_time;
    const int update_ms = 40;
    int period_ms = 4000;
    float pan, tilt;
    float pan_change_amp = 45.0;
    float pan_delta = 0, tilt_delta = 0;
    AudioLayer audio;

  public:
    void init(int audio_pin, int tx_pin);
    void start();
    void stop();
    void update();
    void set_period_ms(int ms) { period_ms = ms;}
    void receivebytes(const char* data, uint8_t len);
};

extern Game game;
