#pragma once
#include "fraise.h"
#include "sound/osc.h"
#include "sound_command.h"

/*class MainPatch {
  private:
  public:
    void buzz() {
    }
    void bounce(bool way = true) {
    }
};*/

class AudioLayer {
  public:
    MainPatch main_patch;
    void init(int audio_pin);
    //void update(){};
    //void audio_task();
    void receivebytes(const char* data, uint8_t len){};
    //void print_cpu();
    void command(SoundCommand c, int p1 = 0, int p2 = 0, int p3 = 0);
};


