// DMX projector

#include "fraise.h"
#include "proj.h"
//#include <math.h>
#include "config.h"

extern unsigned char dmxBuf[];

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))
void proj_goto(float pan, float tilt) {
    pan = CLIP(pan, 0.0, 360.0);
    tilt = CLIP(tilt, -90.0, 90.0);
    unsigned int real_pan = /*(360.0 - pan)*/pan * (config.proj_pan_amp / 360.0);
    unsigned int real_tilt = tilt * (27330 / 90.0) + 32768U;
    dmxBuf[PROJ_DMX_OFFSET + 0] = real_pan / 256U;
    dmxBuf[PROJ_DMX_OFFSET + 1] = real_pan % 256U;
    dmxBuf[PROJ_DMX_OFFSET + 2] = real_tilt / 256U;
    dmxBuf[PROJ_DMX_OFFSET + 3] = real_tilt % 256U;
    //printf("l %f: %d : %d %d | %f: %d : %d %d\n", pan, real_pan, real_pan / 256U, real_pan % 256U, tilt, real_tilt, real_tilt / 256U, real_tilt % 256U);
    fraise_put_init();
    fraise_put_uint8(101);
    fraise_put_int16(pan * 32);
    fraise_put_int16(tilt * 32);
    fraise_put_send();
}

void proj_set_light(uint8_t l) {
    dmxBuf[PROJ_DMX_OFFSET + 7] = l;
}
