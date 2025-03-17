// Lidar firmware

#define BOARD pico
#include "fraise.h"
#include "hardware/uart.h"
#include "string.h"
#include "fraise_eeprom.h"

#include "hw/dmx.h"
#include "hw/lidar.h"
#include "hw/proj.h"
#include "hw/pixel.h"
#include "hw/logger.h"
#include "hw/cpuload.h"
#include "game/players.h"
#include "game/game.h"
#include "sound/audiolayer.h"
#include "config.h"
#include "pico/rand.h"

#define printf fraise_printf

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

CpuLoad pixel_load("pixel");
CpuLoad lidar_load("lidar");
CpuLoad game_load("game");

Logger scorelog;
AudioLayer audio;

Game game(scorelog, audio);

void setup() {
    eeprom_load();

    scorelog.init(LOGGER_START, LOGGER_SIZE);

    audio.init(AUDIO_PWM_PIN);
    game.init(MP3_TX_PIN);

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

    srandom(get_rand_32());
    setup_lidar(LIDAR_TX_PIN, LIDAR_RX_PIN, LIDAR_UART);
    sleep_ms(800);
    lidar_change_state(START);

    proj.color(DMXProj::white);
}

void game_pixels_update() {
    game.pixels_update();
}

void send_positions() {
    fraise_put_init();
    fraise_put_uint8(100);
    fraise_put_send();
    for(int i : game.players.get_object_set()) {
        if(!game.players.is_object_visible(i)) continue;
        Position &p = game.players.get_object_pos(i);
        if(&p != &null_position) {
            fraise_put_init();
            fraise_put_uint8(110);
            fraise_put_uint8(i);
            fraise_put_uint16(p.angle);
            fraise_put_uint16(p.distance);
            fraise_put_uint16(p.size);
            fraise_put_send();
        }
    }
    for(int i : game.players.get_set()) {
        if(!game.players.is_visible(i)) continue;
        Position &p = game.players.get_pos(i);
        if(&p != &null_position) {
            fraise_put_init();
            fraise_put_uint8(111);
            fraise_put_uint8(i);
            fraise_put_uint16(p.angle);
            fraise_put_uint16(p.distance);
            fraise_put_uint16(p.size);
            fraise_put_send();
        }
    }
}

void loop() {
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

    lidar_load.start();
    if(lidar_update()) {
        // update players
        //game.players.find_players(lidar_distance_masked);
        game.players.find_players_light(lidar_distance_masked);
        //send_positions();
        lidar_load.stop();
    }

    game_load.start();
    if(game.update()) game_load.stop();

    pixel_load.start();
    if(pixel_update()) {
        game.pixels_update();
        pixel_load.stop();
    }
    //pixel_update(game_pixels_update);

    if(dmx.transfer_finished()) {
        dmx.transfer_frame(dmxBuf, DMX_CHAN_COUNT);
    }
}

void fraise_receivebytes(const char *data, uint8_t len) {
    uint8_t command = fraise_get_uint8();
    //fraise_printf("l get command %d\n", command);
    switch(command) {
    case 1:
        ledPeriod = (int)fraise_get_uint8() * 10;
        break;
    case 4:
    {
        int c2 = fraise_get_uint8();
        switch(c2) {
        case 0:
            lidar_reset();
            break;
        case 1:
            lidar_stop();
            lidar_change_state(STOP);
            break;
        case 2:
            lidar_start();
            lidar_change_state(RUN);
            break;
        case 3:
            lidar_background_snap();
            break;
        case 4:
            lidar_change_state(SNAP_PRE);
            break;
        case 5:
            lidar_print_status();
            break;
        default:
            ;
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

    case 10:
        pixel_receivebytes(data + 1, len - 1);
        break;
    case 11:
        pixel_load.get_load();
        lidar_load.get_load();
        game_load.get_load();
        break;
    case 12:
        pixel_load.reset();
        lidar_load.reset();
        game_load.reset();
        break;

    case 20:
        config.receivebytes(data + 1, len - 1);
        break;

    case 30:
        game.receivebytes(data + 1, len - 1);
        break;
    case 31:
        game.speaker.receivebytes(data + 1, len - 1);
        break;
    case 32:
        audio.receivebytes(data + 1, len - 1);
        break;
    case 33:
        audio.command((SoundCommand)fraise_get_uint8(),
            fraise_get_int16(), fraise_get_int16(), fraise_get_int16());
        break;
    case 40:
        printf("l logcount %d\n", scorelog.get_count());
        break;
    case 41:
        {
            int num = fraise_get_uint32();
            printf("l logscore %d %d\n", num, scorelog.get_score(num));
        }
        break;
    case 42:
        {
            int score = fraise_get_uint16();
            printf("l new score %d %d/%d\n", score, scorelog.get_rank(score), scorelog.get_count());
            scorelog.write(score);
        }
        break;

    case 100 :
        fraise_print_status();
        break;

    case 102:
        bg_substract = fraise_get_uint16();
        break;
    case 103:
        bg_min_width = fraise_get_uint16();
        break;
    case 104:
        snap_smooth = fraise_get_uint16();
        break;

    case 149:
        game.standby();
        break;
    case 150:
        game.stop();
        break;
    case 151:
        game.start();
        break;
    //case 152: game.set_period_ms(fraise_get_uint16()); break;
    case 153:
        game.prepare();
        break;
    case 154:
        game.players.find_players(lidar_distance_masked);
        break;
    case 155:
        send_positions();
        break;

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
    case 203:
        dmx.reset();
        break;
    case 204:
        dmx.status();
        break;
    case 205: proj.dimmer(fraise_get_uint8()); break;
    case 206:
    {
        float gain_pan = fraise_get_uint16() / 65536.0;
        float gain_tilt = fraise_get_uint16() / 65536.0;
        ((EVOBeam&)proj).set_gains(gain_pan, gain_tilt);
    }
    break;
    default:
        printf("rcvd ");
        for(int i = 0; i < len; i++) printf("%d ", (uint8_t)data[i]);
        putchar('\n');
    }
}

bool is_string(const char *in, uint8_t len, const char *str) {
    return len >= strlen(str) && !strncmp(in, str, strlen(str));
}

void fraise_receivechars(const char *data, uint8_t len) {
    const char eesave_str[] = "SAVE_EEPROM";
    const char clearlog_str[] = "CLEAR_LOG";
    if(data[0] == 'E') { // Echo
        printf("E%s\n", data + 1);
    }
    else if(is_string(data, len, eesave_str) /*len >= strlen(eesave_str) && !strncmp(data, eesave_str, strlen(eesave_str))*/) {
        printf("l saving eeprom\n");
        lidar_stop();
        sleep_ms(500);
        eeprom_save();
        lidar_start();
    }
    else if(is_string(data, len, clearlog_str)) {
        printf("l clearing log\n");
        scorelog.clear_all();
    }
}

void eeprom_declare_main() {
    eeprom_declare_data((char*)lidar_background, sizeof(lidar_background));
    config.eeprom_declare();
}

