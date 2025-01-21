// Lidar firmware

#define BOARD pico
#include "fraise.h"
#include "hardware/uart.h"
#include "string.h"
#include "fraise_eeprom.h"
#include "dmx.h"
//#include "wavplayer.h"
#include "lidar.h"
#include "players.h"
#include "proj.h"
#include "game.h"
#include "pixel.h"
//#include "piotx.h"
//#include "sound.h"
#include "osc.h"
#include "config.h"
//#include "../wavs_duration.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
int ledPeriod = 250;

const uint AUDIO_PWM_PIN = 10;
const uint AUDIO_GND_PIN = 11;

const uint BUTTON_PIN = 14;

const uint LIDAR_TX_PIN = 12;
const uint LIDAR_RX_PIN = 13;
uart_inst_t *LIDAR_UART = uart0;

const int DMX_TX_PIN = 4;
const int DMX_DRV_PIN = 5;
uart_inst_t *DMX_UART = uart1;
#define DMX_CHAN_COUNT 256
unsigned char dmxBuf[DMX_CHAN_COUNT];
DmxMaster dmx;

const int MP3_TX_PIN = 8;

bool button, button_last;
int button_count;

//Osc osc1;

void setup() {
    game.init(AUDIO_PWM_PIN, MP3_TX_PIN);
    Osc::setup();
    /*osc1.setFreq(440);
    osc1.setVol(0);*/

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    gpio_init(AUDIO_GND_PIN);
    gpio_set_dir(AUDIO_GND_PIN, GPIO_OUT);
    gpio_put(AUDIO_GND_PIN, 0);

    dmx.init(DMX_UART, DMX_TX_PIN);
    gpio_init(DMX_DRV_PIN);
    gpio_set_dir(DMX_DRV_PIN, GPIO_OUT);
    gpio_put(DMX_DRV_PIN, 0);

    pixel_setup();

    setup_lidar(LIDAR_TX_PIN, LIDAR_RX_PIN, LIDAR_UART);
    sleep_ms(200);
    lidar_state = START;

    eeprom_load();
}

void loop(){
	static absolute_time_t nextLed;
	static bool led = false;

	if(time_reached(nextLed)) {
		gpio_put(LED_PIN, led = !led);
		nextLed = make_timeout_time_ms(ledPeriod);
	}

	if(gpio_get(BUTTON_PIN)) {
		if(button_count > 0) button_count--;
		else button = false;
	} else {
		if(button_count < 1000) button_count++;
		else button = true;
	}
	if(button_last != button) {
		button_last = button;
		printf("b %d\n", button);
		if(button) {
			//lidar_state = SNAP_PRE;
		}
	}

	if(lidar_update()) {
        players_find(lidar_distance_masked);
        fraise_put_init();
        fraise_put_uint8(100);
        for(int i = 0; i < players_count; i++) fraise_put_uint16(players_pos[i]);
        fraise_put_send();
        //pixel_update_players(players_count, players_pos, players_separation);
        game.pixel_update_players();
    }
    game.update();

    if(dmx.transfer_finished()) {
        dmx.transfer_frame(dmxBuf, DMX_CHAN_COUNT);
    }
}

void fraise_receivebytes(const char *data, uint8_t len){
	uint8_t command = fraise_get_uint8();

	switch(command) {
	    case 1: ledPeriod = (int)fraise_get_uint8() * 10; break;
	    case 4:
	        {
		        int c2 = fraise_get_uint8();
		        switch(c2) {
			        case 0: lidar_reset(); break;
			        case 1: lidar_stop(); lidar_state = STOP; break;
			        case 2: lidar_start(); lidar_state = RUN; break;
			        case 3: lidar_background_snap(); break;
			        case 4: lidar_state = SNAP_PRE; break;
			        case 5: lidar_print_status(); break;
			        default: ;
		        }
	        }
	        break;
	    case 5:
		    for(int a = 0; a < 36 ; a++) {
			    fraise_put_init();
			    fraise_put_uint8(20);
			    fraise_put_uint8(a);
			    for(int i = 0; i < 10; i++) fraise_put_uint16(lidar_distance[a * 10 + i]);
			    fraise_put_send();
		    }
		    break;
	    case 6:
		    for(int a = 0; a < 36 ; a++) {
			    fraise_put_init();
			    fraise_put_uint8(21);
			    fraise_put_uint8(a);
			    for(int i = 0; i < 10; i++) fraise_put_uint16(lidar_background[a * 10 + i]);
			    fraise_put_send();
		    }
	        break;
	    case 7:
		    for(int a = 0; a < 36 ; a++) {
			    fraise_put_init();
			    fraise_put_uint8(22);
			    fraise_put_uint8(a);
			    for(int i = 0; i < 10; i++) fraise_put_uint16(lidar_distance_masked[a * 10 + i]);
			    fraise_put_send();
		    }
	        break;

        case 10: pixel_receivebytes(data + 1, len - 1); break;

        case 20: config.receivebytes(data + 1, len - 1); break;

        case 30: game.receivebytes(data + 1, len - 1); break; //mp3_play(fraise_get_uint16()); break;
        /*case 40: {
                for(int i = 0; i < 32; i++) printf("s %d\n", osc1.getSample());
            }
            break;
        case 41: osc1.setFreq(fraise_get_uint16()); break;
        case 42: osc1.setVol(fraise_get_uint16()); break;
        case 43: osc1.setStep(fraise_get_uint32()); break;*/

	    case 100 : fraise_print_status(); break;
	    case 102: bg_substract = fraise_get_uint16(); break;
	    case 103: bg_min_width = fraise_get_uint16(); break;
	    case 104: snap_smooth = fraise_get_uint16(); break;
	    case 150: game.stop(); break;
	    case 151: game.start(); break;
	    //case 152: game.set_period_ms(fraise_get_uint16()); break;
	    case 153: game.prepare(); break;

	    case 200:
	        {
	            int chan = fraise_get_uint16();
	            int val = fraise_get_uint8();
	            dmxBuf[chan] = val;
	        }
	        break;
	    case 201:
	        {
	            float pan = fraise_get_uint16() / 32.0;
	            float tilt = fraise_get_int16() / 32.0;
	            proj.move(pan, tilt);
	        }
	        break;
	    case 202:
	        {
	            int chan = fraise_get_uint16();
	            printf("202 %d %d\n", chan, dmxBuf[chan]);
	        }
	        break;
        case 203: dmx.reset(); break;
        case 204: dmx.status(); break;
	    default:
		    printf("rcvd ");
		    for(int i = 0; i < len; i++) printf("%d ", (uint8_t)data[i]);
		    putchar('\n');
	}
}

void fraise_receivechars(const char *data, uint8_t len){
	const char eesave_str[] = "SAVE_EEPROM";
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
	else if(len >= strlen(eesave_str) && !strncmp(data, eesave_str, strlen(eesave_str))) {
	    printf("l saving eeprom\n");
		lidar_stop();
		sleep_ms(500);
		eeprom_save();
		lidar_start();
    }
}

void eeprom_declare_main() {
	eeprom_declare_data((char*)lidar_background, sizeof(lidar_background));
	config.eeprom_declare();
}

