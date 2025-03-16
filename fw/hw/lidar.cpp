// RPlidar C1-M1

#include "fraise.h"
#include "lidar.h"
#include "string.h"
#include "config.h"

lidar_state_t lidar_state;
uint16_t lidar_distance[360];
uint16_t lidar_distance_min[360];
uint8_t lidar_written[360];
uint8_t lidar_min_written[360];

uint16_t lidar_background[360];
uint16_t lidar_distance_masked[360];
bool lidar_header_ok;
uint8_t lidar_buffer[10];
uint8_t lidar_bufcount;
const uint8_t lidar_header_ref[] = {0xA5, 0x5A, 0x05, 0x00, 0x00, 0x40, 0x81};
uart_inst_t *lidar_uart;

#define DISTANCE_FAR 12000

int bg_substract = 100;
float bg_compress = 0.9;
int bg_min_width = 1;
int snap_smooth = 2;

#define CLIP(x, min, max) MAX(min, MIN(max, x))

// irq handler that stores the distance map
void lidar_irq() {
    while(uart_is_readable(lidar_uart)) {
        uint8_t b = uart_getc(lidar_uart);
        if(lidar_bufcount < 10) lidar_buffer[lidar_bufcount++] = b;
        if(! lidar_header_ok) {
            if(
                (lidar_bufcount == 7) &&
                (memcmp(lidar_buffer, lidar_header_ref, sizeof(lidar_header_ref)) == 0)
            ) {
                lidar_header_ok = true;
                lidar_bufcount = 0;
            }
        } else {
            if(lidar_bufcount == 5) {
                int a = ((lidar_buffer[1] + lidar_buffer[2] * 128) / 64) % 360;
                int d = (lidar_buffer[3] + lidar_buffer[4] * 256) / 4;
                if(d < config.distance_min) d  = DISTANCE_FAR;
                if(d) {
                    if(lidar_min_written[a]) lidar_distance_min[a] = MIN(lidar_distance_min[a], d);
                    else lidar_distance_min[a] = d;
                    lidar_distance[a] = d;
                    lidar_min_written[a] = lidar_written[a] = 1;
                }
                lidar_bufcount = 0;
            }
        }
    }
}

void lidar_reset() {
    uart_putc_raw(lidar_uart, 0xA5);
    uart_putc_raw(lidar_uart, 0x40);
}

void lidar_start() {
    uart_putc_raw(lidar_uart, 0xA5);
    uart_putc_raw(lidar_uart, 0x20);
    lidar_header_ok = false;
    lidar_bufcount = 0;
}

void lidar_stop() {
    uart_putc_raw(lidar_uart, 0xA5);
    uart_putc_raw(lidar_uart, 0x25);
}

void lidar_background_snap() {
    for(int a = 0; a < 360; a++) {
        int d = 12000;
        for(int o = -snap_smooth; o <= snap_smooth ; o++) {
            int ao = (a + o + 360) % 360;
            int dist = lidar_distance_min[ao];
            if(dist < config.distance_min) dist  = DISTANCE_FAR;
            if(dist < d) d = dist;
        }
        if(d > bg_substract)
            d = (d - bg_substract) * bg_compress;
        else
            lidar_background[a] = 0;
        lidar_background[a] = d;
    }
    // acknowledge the reading of min_data
    memset(lidar_min_written, 0, sizeof(lidar_min_written));
}

void setup_lidar(int tx_pin, int rx_pin, uart_inst_t *uart) {
    lidar_uart = uart;
    uart_init(lidar_uart, 460800);
    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = lidar_uart == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, lidar_irq);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(lidar_uart, true, false);

    //lidar_maxing = false;
    lidar_state = STOP;
}

void lidar_adapt_background() {
    for(int a = 0; a < 360; a++) {
        if(lidar_min_written[a]) {
            int min_dist = DISTANCE_FAR;
            for(int o = -snap_smooth; o <= snap_smooth ; o++) {
                int ao = (a + o + 360) % 360;
                int dist = lidar_distance_min[ao];
                if(dist < config.distance_min) dist  = DISTANCE_FAR;
                if(dist < min_dist) min_dist = dist;
            }
            if((min_dist - bg_substract) > lidar_background[a] && lidar_background[a] < DISTANCE_FAR) lidar_background[a] += 50;
            else if(lidar_background[a] > 10) lidar_background[a] -= 10;
        }
    }
    // acknowledge the reading of min_data
    memset(lidar_min_written, 0, sizeof(lidar_min_written));
}

bool lidar_update() {
    static absolute_time_t update_time = get_absolute_time();
    if(!time_reached(update_time)) return false;
    update_time = make_timeout_time_ms(100);
    switch(lidar_state) {
    case STOP:
        break;
    case START:
        lidar_start();
        update_time = make_timeout_time_ms(1000);
        lidar_state = RUN;
        break;
    case RUN:
    {
        // mask the distance map with the background map;
        for(int a = 0; a < 360; a++) {
            if(!lidar_written[a]) lidar_distance_masked[a] = DISTANCE_FAR;
            else
            {
                int dist = lidar_distance[a];
                if(dist > 0 && dist < lidar_background[a] && dist > config.distance_min) {
                    lidar_distance_masked[a] = dist;
                } else lidar_distance_masked[a] = DISTANCE_FAR;
            }
        }
        // acknowledge the reading of data, we don't need lidar_distance[] anymore
        memset(lidar_written, 0, sizeof(lidar_written));

        static int count = 0;
        if(count == 10) {
            lidar_adapt_background();
            count = 0;
        }
        else count++;

        // discard objects that are too narrow:
        for(int a = 0; a < 360; a++) {
            if(lidar_distance_masked[a] < config.distance_max) {
                for(int o = 0; o < bg_min_width; o++) {
                    if(lidar_distance_masked[(a + o) % 360] > config.distance_max) {
                        lidar_distance_masked[a] = DISTANCE_FAR;
                        break;
                    }
                }
            }
        }
        return true;
    }
    break;
    case SNAP_PRE:
        printf("l snap init\n");
        update_time = make_timeout_time_ms(4000);
        lidar_state = SNAP;
        break;
    case SNAP:
        printf("l snap start\n");
        update_time = make_timeout_time_ms(1000);
        memset(lidar_min_written, 0, sizeof(lidar_min_written));
        lidar_state = SNAP_POST;
        break;
    case SNAP_POST:
        lidar_background_snap();
        lidar_state = RUN;
        printf("l snap done\n");
        break;
    default:
        ;
    }
    return false;
}

void lidar_print_status() {
    printf("l lidar state %d\n", (int)lidar_state);
}

void lidar_change_state(lidar_state_t s) {
    lidar_state = s;
}

