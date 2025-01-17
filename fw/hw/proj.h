// DMX projector

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EXTERN_P
#define EXTERN_P extern
#endif

#define PROJ_DMX_OFFSET 1

void proj_goto(float pan, float tilt);
void proj_set_light(uint8_t l);

#ifdef __cplusplus
}
#endif

