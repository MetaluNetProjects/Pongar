// audio layer

#pragma once
#include "fraise.h"
#include "wavplayer.h"
#include "osc.h"

class AudioLayer {
  private:
    WavPlayer player;
    float cpu_avg;
  public:
    MainPatch main_patch;
    void init(int audio_pin, int tx_pin);
    void update();
    void audio_task();
    void receivebytes(const char* data, uint8_t len);
    void print_cpu();
};
