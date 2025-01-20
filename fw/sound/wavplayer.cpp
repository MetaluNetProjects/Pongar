// MP3 player

#include "fraise.h"
#include "wavplayer.h"
#include "piotx.h"

PioTx mp3tx;
#define PUTC(c) mp3tx.putc(c)

void mp3_play(uint index) {
	PUTC(0x7E);
	PUTC(0xFF);
	PUTC(0x06); // command len
	PUTC(0x03); // play track
	PUTC(0x00); // no reply
	PUTC(index >> 8);
	PUTC(index & 0xFF);
	//PUTC(255 - (index >> 8));
	//PUTC(255 - (index & 0xFF));
	PUTC(0xEF);
}

void mp3_folder_track(uint8_t folder, uint8_t track) {
	if(folder > 99) folder = 99;
	if(folder < 1) folder = 1;
	PUTC(0x7E);
	PUTC(0xFF);
	PUTC(0x06); // command len
	PUTC(0x0F); // folder track
	PUTC(0x00); // no reply
	PUTC(folder);
	PUTC(track);
	PUTC(0xEF);
}

void mp3_volume(uint volume) {
	if(volume > 30) volume = 30;
	PUTC(0x7E);
	PUTC(0xFF);
	PUTC(0x06);
	PUTC(0x06);
	PUTC(0x00);
	PUTC(0x00);
	PUTC(volume);
	PUTC(0xEF);
}

void setup_mp3(uint pin) {
	mp3tx.init(pin, 9600);
	//gpio_set_function(MP3_TX_PIN, GPIO_FUNC_UART);
	//gpio_set_function(MP3_RX_PIN, GPIO_FUNC_UART);
}

void mp3_print_status() {
    mp3tx.print_status();
}

void mp3_putc(char c) {
    mp3tx.putc(c);
}

//-----------------------------
#include "../config.h"

RomTable<uint16_t> WavPlayer::wavsDuration;

void WavPlayer::init(int tx_pin) {
    setup_mp3(tx_pin);
    wavsDuration.init(WAVDUR_TABLE_START, WAVDUR_TABLE_COUNT);
}

void WavPlayer::play(uint8_t folder, uint8_t track) {
    if(waiting.size() >= max_waiting) return;
    waiting.push_back((folder << 16) + track);
}

void WavPlayer::silence(uint16_t ms) {
    if(waiting.size() >= max_waiting) return;
    waiting.push_back(0xff000000 + ms);
}

void WavPlayer::clear() {
    waiting.clear();
}

bool WavPlayer::is_playing() {
    return !(time_reached(end_of_play) && waiting.empty());
}

void WavPlayer::update() {
    if((!time_reached(end_of_play)) || waiting.empty()) return;
    uint32_t next = waiting.front();
    waiting.pop_front();
    uint8_t command = next >> 24;
    int duration_ms = 1;
    switch(command) {
        case 0:
            {
                uint8_t folder = next >> 16;
                if(folder > 99) folder = 99;
                if(folder < 1) folder = 1;
                uint8_t track = next & 255;
                mp3_folder_track(folder, track);
                duration_ms = wavsDuration.get_item(((int)folder - 1) * 256 + track) * 10 + 50;
            }
            break;
        case 255: duration_ms = next & 0xffff; break;
    }
    end_of_play = make_timeout_time_ms(duration_ms);
}

void WavPlayer::receivebytes(const char* data, uint8_t len) {
    char command = fraise_get_uint8();
    //printf("player rcv %d\n", command);
    switch(command) {
        case 10: wavsDuration.copy_flash_to_ram(); break;
        case 11:
            {
                int start = fraise_get_uint16();
                uint16_t val;
                for(int i = 0; i < 10; i++) {
                    val = fraise_get_uint16();
                    wavsDuration.record_item_to_ram(val, start + i);
                }
            }
            break;
        case 12: wavsDuration.save_ram_to_flash(); break;
        case 13: 
            {
                uint16_t index = fraise_get_uint16();
                uint16_t res = wavsDuration.get_item(index);
                printf("l wavdur[%d]=%d\n", index, res);
            }
            break;
        case 30: mp3_play(fraise_get_uint16()); break;
        case 31: mp3_volume(fraise_get_uint8()); break;
        case 32: mp3_print_status(); break;
        case 33: mp3_putc(fraise_get_uint8()); break;
        case 34: mp3_folder_track(fraise_get_uint8(), fraise_get_uint8()); break;
        case 35:
            {
                int8_t folder = fraise_get_uint8();
                int16_t track = fraise_get_int16();
                if(track < 0) silence(-track);
                else play(folder, track);
            }
            break;
        case 50: printf("l playing %d\n", is_playing()); break;
    }
}
