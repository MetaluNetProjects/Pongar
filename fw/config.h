// Pongar config

#pragma once

class PongarConfig {
private:
    void pixel_show(int from, int to);
public:
    void eeprom_declare();
    void receivebytes(const char* data, uint8_t len);
    // data:
    uint16_t ring_leds = 234;
    int16_t leds_angle_offset = 0; //112;

    int16_t distance_max = 1700;
    int16_t distance_min = 400;
    int16_t lidar_angle_offset = 0;

    int16_t proj_tilt_amp = 45;
    uint16_t proj_pan_amp = 44850;

    int16_t game_distance_min = 700;

    uint8_t volume = 120;
    uint8_t spot_leds = 4;

    uint8_t proj_lum = 150;

    uint8_t disable_wait_stable = 0;
    uint8_t disable_too_close = 0;
    uint8_t disable_too_close_alarm = 0;
    uint8_t disable_too_many = 0;
};

extern PongarConfig config;

#define CEILING(x,y) (((x) + (y) - 1) / (y))
#define ROUND4K(x) (CEILING(x, 4096) * 4096)

#define WAVDUR_TABLE_COUNT (100 * 256) // 51200 bytes = 12.5 * 4k
#define WAVDUR_TABLE_FLASHSIZE ROUND4K(WAVDUR_TABLE_COUNT * 2)
//static_assert(WAVDUR_TABLE_FLASHSIZE == 13 * 4096); // assert wavdur_table fits in 13 blocks.
#define WAVDUR_TABLE_START (XIP_BASE + PICO_FLASH_SIZE_BYTES - WAVDUR_TABLE_FLASHSIZE)

#define LOGGER_SIZE (1024 * 1024) // 1MB: 524288 x 16bits scores
#define LOGGER_START (WAVDUR_TABLE_START - LOGGER_SIZE)
//static_assert((LOGGER_START - XIP_BASE) / 1024 > 900); // assert there is at least 900kB left in flash.

