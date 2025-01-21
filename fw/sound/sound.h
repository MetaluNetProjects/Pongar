// audio layer

#pragma once
#include "fraise.h"
//#include "wavplayer.h"
#include "osc.h"
#include "sound_command.h"

class AudioLayer {
  private:
    //WavPlayer player;
    float cpu_avg;
  public:
    MainPatch main_patch;
    void init(int audio_pin/*, int tx_pin*/);
    
    //void update();
    void audio_task();
    void receivebytes(const char* data, uint8_t len);
    void print_cpu();
    void command(SoundCommand c, int p1 = 0, int p2 = 0, int p3 = 0);
    //bool player_is_playing();
};
