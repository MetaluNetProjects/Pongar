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
    int duration_offset = 0;
    uint8_t last_command = 0;
public:
    void init(int tx_pin);
    virtual void reload_durations() {}
    void play(uint8_t folder, uint8_t track);
    void silence(uint16_t ms);
    void clear();
    bool is_playing();
    bool is_really_playing(); // is playing a real wav (not silence)
    int get_duration_ms(uint8_t folder, uint8_t track);
    void update();
    void receivebytes(const char* data, uint8_t len);
    void set_volume(uint8_t vol);
};


