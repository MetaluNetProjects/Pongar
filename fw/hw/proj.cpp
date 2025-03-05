// DMX projector

#include "fraise.h"
#include "proj.h"
#include "config.h"

extern unsigned char dmxBuf[];

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

/*void proj_goto(float pan, float tilt) {
    pan = CLIP(pan, 0.0, 360.0);
    tilt = CLIP(tilt, -90.0, 90.0);
    unsigned int real_pan = pan * (config.proj_pan_amp / 360.0);
    unsigned int real_tilt = tilt * (27330 / 90.0) + 32768U;
    dmxBuf[PROJ_DMX_OFFSET + 0] = real_pan / 256U;
    dmxBuf[PROJ_DMX_OFFSET + 1] = real_pan % 256U;
    dmxBuf[PROJ_DMX_OFFSET + 2] = real_tilt / 256U;
    dmxBuf[PROJ_DMX_OFFSET + 3] = real_tilt % 256U;
    fraise_put_init();
    fraise_put_uint8(101);
    fraise_put_int16(pan * 32);
    fraise_put_int16(tilt * 32);
    fraise_put_send();
}

void proj_set_light(uint8_t l) {
    dmxBuf[PROJ_DMX_OFFSET + 7] = l;
}*/

void MaxiSpot60::move(float pan, float tilt) {
    pan = CLIP(pan, 0.0, 360.0);
    tilt = CLIP(tilt, -90.0, 90.0);
    unsigned int real_pan = pan * (config.proj_pan_amp / 360.0);
    unsigned int real_tilt = tilt * (27330 / 90.0) + 32768U;
    dmxBuf[dmx_address - 1 + 1] = real_pan / 256U;
    dmxBuf[dmx_address - 1 + 2] = real_pan % 256U;
    dmxBuf[dmx_address - 1 + 3] = real_tilt / 256U;
    dmxBuf[dmx_address - 1 + 4] = real_tilt % 256U;

    fraise_put_init();
    fraise_put_uint8(101);
    fraise_put_int16(pan * 32);
    fraise_put_int16(tilt * 32);
    fraise_put_send();
}

void MaxiSpot60::dimmer(uint8_t l) {
    dmxBuf[dmx_address - 1 + 8] = l;
}
void MaxiSpot60::color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {}

//---------------------------------------------------------

void Movobeam100::move(float pan, float tilt) {
    pan = CLIP(pan, 0.0, 360.0);
    tilt = CLIP(tilt, -90.0, 90.0);
    unsigned int real_pan = pan * (config.proj_pan_amp / 360.0);
    unsigned int real_tilt = tilt * (27330 / 90.0) + 32768U;
    dmxBuf[dmx_address - 1 + 1] = real_pan / 256U;
    dmxBuf[dmx_address - 1 + 2] = real_pan % 256U;
    dmxBuf[dmx_address - 1 + 4] = real_tilt / 256U;
    dmxBuf[dmx_address - 1 + 5] = real_tilt % 256U;

    fraise_put_init();
    fraise_put_uint8(101);
    fraise_put_int16(pan * 32);
    fraise_put_int16(tilt * 32);
    fraise_put_send();
}

void Movobeam100::dimmer(uint8_t l) {
    dmxBuf[dmx_address - 1 + 7] = l;
}

void Movobeam100::color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    dmxBuf[dmx_address - 1 + 9] = r;
    dmxBuf[dmx_address - 1 + 10] = g;
    dmxBuf[dmx_address - 1 + 11] = b;
    dmxBuf[dmx_address - 1 + 12] = w;
}

//---------------------------------------------------------

void Movobeam100Zoom::color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    dmxBuf[dmx_address - 1 + 10] = r;
    dmxBuf[dmx_address - 1 + 11] = g;
    dmxBuf[dmx_address - 1 + 12] = b;
    dmxBuf[dmx_address - 1 + 13] = w;
    zoom(255);
}

void Movobeam100Zoom::zoom(uint8_t z) {
    dmxBuf[dmx_address - 1 + 15] = z;
}

//-----------------------------------------------------------

void ClubSpot::move(float pan, float tilt) {
    pan = CLIP(pan, 0.0, 360.0);
    tilt = CLIP(tilt, -90.0, 90.0);
    unsigned int real_pan = pan * (config.proj_pan_amp / 360.0);
    unsigned int real_tilt = tilt * (27330 / 90.0) + 32768U;
    dmxBuf[dmx_address - 1 + 1] = real_pan / 256U;
    dmxBuf[dmx_address - 1 + 2] = real_pan % 256U;
    dmxBuf[dmx_address - 1 + 3] = real_tilt / 256U;
    dmxBuf[dmx_address - 1 + 4] = real_tilt % 256U;

    fraise_put_init();
    fraise_put_uint8(101);
    fraise_put_int16(pan * 32);
    fraise_put_int16(tilt * 32);
    fraise_put_send();
}

void ClubSpot::dimmer(uint8_t l) {
    dmxBuf[dmx_address - 1 + 6] = l;
}

void ClubSpot::color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    /*dmxBuf[dmx_address - 1 + 9] = r;
    dmxBuf[dmx_address - 1 + 10] = g;
    dmxBuf[dmx_address - 1 + 11] = b;
    dmxBuf[dmx_address - 1 + 12] = w;*/
}

//-----------------------------------------------------------

ClubSpot proj_impl;
DMXProj &proj = proj_impl;

