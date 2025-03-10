// Pongar config

#pragma once

class PongarConfig {
private:
    void pixel_show(int from, int to);
public:
    void eeprom_declare();
    void receivebytes(const char* data, uint8_t len);
    // data:
    uint16_t total_leds = 212;
    int16_t leds_angle_offset = 0; //112;

    int16_t distance_max = 1700;
    int16_t distance_min = 400;
    int16_t lidar_angle_offset = 0;

    int16_t proj_tilt_amp = 45;
    uint16_t proj_pan_amp = 44850;
};

extern PongarConfig config;

#define WAVDUR_TABLE_COUNT (100 * 256)
#define WAVDUR_TABLE_START (XIP_BASE + PICO_FLASH_SIZE_BYTES - (WAVDUR_TABLE_COUNT * 2)) // 51200 bytes = 12.5 * 4k

#ifndef AUDIO_SAMPLE_RATE
#define AUDIO_SAMPLE_RATE 24000 //57439
#endif

#define AUDIO_SAMPLES_PER_BUFFER 64

