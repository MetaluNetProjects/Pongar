#include "fraise.h"
#include "osc.h"
#include "seq.h"

int16_t Osc::sine_wave_table[Osc::sin_table_len];
uint32_t Osc::mtof8_table[Osc::mtof_table_len];

