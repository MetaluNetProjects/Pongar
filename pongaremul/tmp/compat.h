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
absolute_time_t get_absolute_time();

#define MAX(x,m) ((m) > (x) ? (m) : (x))
#define MIN(x,m) ((m) < (x) ? (m) : (x))

uint8_t fraise_get_uint8();

