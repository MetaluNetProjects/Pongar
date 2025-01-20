// Pongar config

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

//#define AUDIO_SAMPLE_RATE 48000 //57439
//#define AUDIO_SAMPLES_PER_BUFFER 64

#include <sys/time.h>
typedef struct timeval absolute_time_t;

absolute_time_t make_timeout_time_ms(int ms);
bool time_reached(absolute_time_t t);
extern absolute_time_t at_the_end_of_time;
#define MAX(x,m) ((m) > (x) ? (m) : (x))
#define MIN(x,m) ((m) < (x) ? (m) : (x))

uint8_t fraise_get_uint8();

/*class PongarConfig {
  public:
    void eeprom_declare();
    void receivebytes(const char* data, uint8_t len);
  // data:
    uint16_t total_leds = 35;
    int16_t leds_angle_offset = 112;

    int16_t distance_max = 1500;
    int16_t distance_min = 400;
    int16_t lidar_angle_offset = 0;

    int16_t proj_tilt_amp = 45;
    uint16_t proj_pan_amp = 44850;
};

extern PongarConfig config;*/

