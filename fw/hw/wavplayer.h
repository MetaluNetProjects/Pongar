// Wav files player

#pragma once

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
    void clear();
    bool is_playing();
    int get_duration_ms(uint8_t folder, uint8_t track);
    void update();
    void receivebytes(const char* data, uint8_t len);
};


