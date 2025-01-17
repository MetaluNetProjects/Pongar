// Pongar config

#pragma once

#include <stdlib.h>
#include <stdint.h>

#define AUDIO_SAMPLE_RATE 48000 //57439
#define AUDIO_SAMPLES_PER_BUFFER 64

#include <sys/time.h>
typedef struct timeval absolute_time_t;

absolute_time_t make_timeout_time_ms(int ms);
bool time_reached(absolute_time_t t);
