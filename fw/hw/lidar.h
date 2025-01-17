// RPlidar C1-M1

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EXTERN_L
#define EXTERN_L extern
#endif


typedef enum {STOP, START, RUN, SNAP_PRE, SNAP, SNAP_POST} lidar_state_t;

EXTERN_L lidar_state_t lidar_state;
EXTERN_L bool lidar_maxing;
EXTERN_L uint16_t lidar_distance[360];
EXTERN_L uint16_t lidar_background[360];
EXTERN_L uint16_t lidar_distance_masked[360];
EXTERN_L uint8_t lidar_written[360];

extern int distance_high;
//extern int distance_low;
extern int bg_substract;
extern float bg_compress;
extern int bg_min_width;
extern int snap_smooth;

void lidar_reset();
void lidar_start();
void lidar_stop();
void lidar_background_snap();
void setup_lidar(int tx_pin, int rx_pin, uart_inst_t *uart);
bool lidar_update();
void lidar_print_status();

#ifdef __cplusplus
}
#endif

