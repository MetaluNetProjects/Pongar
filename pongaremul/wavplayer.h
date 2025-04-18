// Wav files player

#pragma once

#include <list>
#include <fstream>

class WavPlayer {
private:
    uint16_t wavsDuration[100 * 256];
public:
    void init(int tx_pin) {
        std::ifstream in;
        std::string filename = "../wavs_dur_table.txt";
        in.open(filename);
        printf("WavPlayer init: ");
        if (in.is_open()) {
            printf("%s is open\n", filename.c_str());
        } else printf("couldn't open %s\n", filename.c_str());
        int element;
        if (in.is_open()) {
            int i = 0;
            while (in >> element) {
                wavsDuration[i++] = element;
            }
        }
        in.close();
    }

    void play(uint8_t folder, uint8_t track);
    void silence(uint16_t ms);
    void clear();
    bool is_playing();
    int get_duration_ms(uint8_t folder, uint8_t track) {
        return wavsDuration[((int)folder - 1) * 256 + track] * 10;
    }
    void update() {}
    void receivebytes(const char* data, uint8_t len) {}
    void set_volume(uint8_t vol) {}
};

