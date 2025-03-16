// Pongar config

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <sys/time.h>
typedef struct timeval absolute_time_t;

absolute_time_t make_timeout_time_ms(int ms);
bool time_reached(absolute_time_t t);
extern absolute_time_t at_the_end_of_time;
absolute_time_t get_absolute_time();
uint32_t to_ms_since_boot (absolute_time_t t);

#define MAX(x,m) ((m) > (x) ? (m) : (x))
#define MIN(x,m) ((m) < (x) ? (m) : (x))

uint8_t fraise_get_uint8();

