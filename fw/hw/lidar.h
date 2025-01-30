// RPlidar C1-M1

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {STOP, START, RUN, SNAP_PRE, SNAP, SNAP_POST} lidar_state_t;

extern uint16_t lidar_distance_masked[360];
extern uint16_t lidar_distance[360];
extern uint16_t lidar_background[360];

void setup_lidar(int tx_pin, int rx_pin, uart_inst_t *uart);
bool lidar_update();
void lidar_print_status();
void lidar_change_state(lidar_state_t s);

void lidar_reset();
void lidar_start();
void lidar_stop();
void lidar_background_snap();

extern int bg_substract;// = 100;
extern float bg_compress;// = 0.9;
extern int bg_min_width;// = 1;
extern int snap_smooth;// = 2;

#ifdef __cplusplus
}
#endif

