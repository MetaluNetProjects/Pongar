// DMX projector

#include "fraise.h"
#include "proj.h"
#include "config.h"

extern unsigned char dmxBuf[];

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

void DMXProjCompens::move(float pan, float tilt) {
    pan = CLIP(pan, 0.0, 360.0);
    tilt = CLIP(tilt, -90.0, 90.0);

    target_pan = pan * (config.proj_pan_amp / 360.0);
    target_tilt = tilt * (27330 / 90.0) + 32768U;

    fraise_put_init();
    fraise_put_uint8(101);
    fraise_put_int16(pan * 32);
    fraise_put_int16(tilt * 32);
    fraise_put_send();
    if(!compens) {
        dmx_pan = target_pan;
        dmx_tilt = target_tilt;
        sendmove();
    }
}

void DMXProjCompens::update() {
    if(!compens) return;
    estimated_pan = estimated_pan + (target_pan - estimated_pan) * gain_pan;
    dmx_pan = CLIP(target_pan * 2 - estimated_pan, 0, 65535);
    estimated_tilt = estimated_tilt + (target_tilt - estimated_tilt) * gain_tilt;
    dmx_tilt = CLIP(target_tilt * 2 - estimated_tilt, 0, 65535);
    sendmove();
}

void MaxiSpot60::sendmove() {
    dmxBuf[dmx_address - 1 + 1] = dmx_pan / 256U;
    dmxBuf[dmx_address - 1 + 2] = dmx_pan % 256U;
    dmxBuf[dmx_address - 1 + 3] = dmx_tilt / 256U;
    dmxBuf[dmx_address - 1 + 4] = dmx_tilt % 256U;
}

void MaxiSpot60::dimmer(uint8_t l) {
    dmxBuf[dmx_address - 1 + 8] = l;
}

void MaxiSpot60::color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {}
void MaxiSpot60::color(Color col) {
    uint8_t vcol = 0;
    switch(col) {
        case white:     vcol = 0; break;
        case red:       vcol = 18 * 1 + 4; break;
        case blue:      vcol = 18 * 3 + 4; break;
        case orange:    vcol = 126; break;
        case green:     vcol = 18 * 2 + 4; break;
        case yellow:    vcol = 18 * 4 + 4; break;
        //case purple:    vcol = 18 * 5 + 4; break;
    }
    dmxBuf[dmx_address - 1 + 6] = vcol;
}

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

void EVOBeam::sendmove() {
    dmxBuf[dmx_address - 1 + 1] = dmx_pan / 256U;
    dmxBuf[dmx_address - 1 + 2] = dmx_pan % 256U;
    dmxBuf[dmx_address - 1 + 3] = dmx_tilt / 256U;
    dmxBuf[dmx_address - 1 + 4] = dmx_tilt % 256U;
}

void EVOBeam::move(float pan, float tilt) {
    pan = CLIP(pan, 0.0, 360.0);
    tilt = CLIP(tilt, -90.0, 90.0);

    target_pan = pan * (config.proj_pan_amp / 360.0);
    target_tilt = tilt * (27330 / 90.0) + 32768U;

    fraise_put_init();
    fraise_put_uint8(101);
    fraise_put_int16(pan * 32);
    fraise_put_int16(tilt * 32);
    fraise_put_send();
}

void EVOBeam::dimmer(uint8_t l) {
    dmxBuf[dmx_address - 1 + 8] = l;
}

void EVOBeam::color(Color col) {
    uint8_t vcol = 0;
    switch(col) {
        case white: vcol = 0; break;
        case red: vcol = 10; break;
        case blue: vcol = 20; break;
        case orange: vcol = 80; break;
        case green: vcol = 30; break;
        case yellow: vcol = 40; break;
        //case lightpurple: vcol = 60; break;
        //case lightblue: vcol = 70; break;
    }
    dmxBuf[dmx_address - 1 + 5] = vcol;
    dmxBuf[dmx_address - 1 + 7] = 50; // open shutter
}

void EVOBeam::update() {
    estimated_pan = estimated_pan + (target_pan - estimated_pan) * gain_pan;
    dmx_pan = CLIP(target_pan * 2 - estimated_pan, 0, 65535);
    estimated_tilt = estimated_tilt + (target_tilt - estimated_tilt) * gain_tilt;
    dmx_tilt = CLIP(target_tilt * 2 - estimated_tilt, 0, 65535);
    sendmove();
}

//-----------------------------------------------------------

MaxiSpot60 proj_impl;
DMXProj &proj = proj_impl;

