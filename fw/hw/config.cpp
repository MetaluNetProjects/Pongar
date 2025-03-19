// Pongar config

#include "fraise.h"
#include "fraise_eeprom.h"
#include "config.h"
#include "pixel.h"

PongarConfig config;

void PongarConfig::eeprom_declare() {
    eeprom_declare_data((char*)this, sizeof(PongarConfig));
}

void PongarConfig::pixel_show(int from, int to) {
    //from = from % total_leds;
    //to = to % total_leds;
    for(int i = 0; i < NUM_PIXELS - 1; i++) {
        if(i >= from && i <= to) set_pixel(i, 255, 255, 255);
        else set_pixel(i, 0, 0, 0);
    }
}

void PongarConfig::receivebytes(const char* data, uint8_t len) {
    char command = fraise_get_uint8();
    switch(command) {
    case 1: {
        total_leds = fraise_get_uint16();
        pixel_show(0, total_leds - 1);
    }
    break;
    case 2: {
        leds_angle_offset = fraise_get_int16();
        int led = (((leds_angle_offset + 360) % 360) * total_leds) / 360;
        pixel_show(led, led);
    }
    break;
    case 3:
        distance_max = fraise_get_int16();
        break;
    case 4:
        distance_min = fraise_get_int16();
        break;
    case 5:
        lidar_angle_offset = fraise_get_int16();
        break;
    case 6:
        proj_tilt_amp = fraise_get_int16();
        break;
    case 7:
        proj_pan_amp = fraise_get_uint16();
        break;
    case 8:
        game_distance_min = fraise_get_uint16();
        break;
    case 100:
        printf("cfg %d %d %d %d %d %d %d\n",
               total_leds, leds_angle_offset,
               distance_max, distance_min,
               lidar_angle_offset, proj_tilt_amp, proj_pan_amp);
        break;
    case 101:
        fraise_put_init();
        fraise_put_uint8(200);
        fraise_put_int16(total_leds);
        fraise_put_int16(leds_angle_offset);
        fraise_put_int16(distance_max);
        fraise_put_int16(distance_min);
        fraise_put_int16(lidar_angle_offset);
        fraise_put_int16(proj_tilt_amp);
        fraise_put_uint16(proj_pan_amp);
        fraise_put_uint16(game_distance_min);
        fraise_put_send();
        break;
    }
}

