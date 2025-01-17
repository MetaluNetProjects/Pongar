/**
 * Simple blinking fruit
 */

#define BOARD pico
#include "fraise.h"
#include "dmx.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
const int DMX_PIN = 4;
const int DMX_DRV_PIN = 5;
uart_inst_t *DMX_UART = uart1;

int ledPeriod = 250;

#define DMX_CHAN_COUNT 256
char dmxBuf[DMX_CHAN_COUNT];

absolute_time_t fill_frame_start_time;
int fill_frame_duration_us;
int frame_duration_us;
float cpuload = 0; // %

DmxMaster dmx(DMX_UART, DMX_PIN);

void setup() {
    dmx.init();
    gpio_init(DMX_DRV_PIN);
    gpio_set_dir(DMX_DRV_PIN, GPIO_OUT);
    gpio_put(DMX_DRV_PIN, 0);
}

void loop(){
	static absolute_time_t nextLed;
	static bool led = false;

    if(dmx.transfer_finished()) {
        dmx.transfer_frame(dmxBuf, DMX_CHAN_COUNT);
    }

	if(time_reached(nextLed)) {
		gpio_put(LED_PIN, led = !led);
		nextLed = make_timeout_time_ms(ledPeriod);
	}
}

void fraise_receivebytes(const char *data, uint8_t len){
	int command = fraise_get_uint8();
	if(command == 1) ledPeriod = fraise_get_uint8() * 10;
	else if(command == 100) {
	    int chan = fraise_get_uint16();
	    int val = fraise_get_uint8();
	    dmxBuf[chan] = val;
	}
}

void fraise_receivechars(const char *data, uint8_t len){
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
	else if(data[0] == 'L') { // get load
		printf("L %f\n", cpuload);
	}
}

