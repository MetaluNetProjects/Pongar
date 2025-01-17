// Players position

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EXTERN_P
#define EXTERN_P extern
#endif

#define PLAYERS_MAX 20
extern int players_separation; // 30
EXTERN_P int players_count;
EXTERN_P uint16_t players_pos[PLAYERS_MAX];

void players_find(const uint16_t *distance_array);

#ifdef __cplusplus
}
#endif

