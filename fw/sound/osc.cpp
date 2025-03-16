#include "fraise.h"
#include "osc.h"

int16_t Osc::sine_wave_table[Osc::sin_table_len];
uint32_t Osc::mtof8_table[Osc::mtof_table_len];
int16_t Blosc::transition_table[Blosc::transition_table_len];
