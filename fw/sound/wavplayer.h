// Wav files player

#pragma once

/*
#ifdef __cplusplus
extern "C" {
#endif


void mp3_putc(char c);
void mp3_play(uint index);
void mp3_folder_track(uint8_t folder, uint8_t track);
void mp3_volume(uint volume);
void setup_mp3(uint pin);
void mp3_print_status();
#ifdef __cplusplus
}
#endif
*/

#include <list>
#include "romtable.h"

class WavPlayer {
  private:
    static const int max_waiting = 32;
    std::list<uint32_t> waiting;
    absolute_time_t end_of_play;
    static RomTable<uint16_t> wavsDuration;
  public:
    void init(int tx_pin);
    void play(uint8_t folder, uint8_t track);
    void silence(uint16_t ms);
    void update();
    void receivebytes(const char* data, uint8_t len);
};


