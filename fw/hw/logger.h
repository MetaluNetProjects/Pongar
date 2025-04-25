// Flash logger

#pragma once

#include "game/scorelog.h"
#include "hardware/flash.h"
#include "pico/sync.h"
#include "fraise_eeprom.h" // for locking out the other core
#include "lidar.h"

class Logger: public Scorelog {
  private:
    intptr_t start_address;
    uint32_t size;
    static const unsigned int event_size = 2;
    unsigned int event_count;

  public:
    void init(intptr_t _start_address, uint32_t _size) {
        start_address = _start_address;
        size = _size;
        get_rank(0); // init event_count
    }

    virtual void clear_all() {
        lidar_stop();
        sleep_ms(500);
        lockout_other_core();
        uint32_t status = save_and_disable_interrupts();
        flash_range_erase(start_address - (intptr_t)XIP_BASE, size);
        restore_interrupts(status);
        unlockout_other_core();
        lidar_start();
        event_count = 0;
    }

    virtual uint16_t get_score(unsigned int num) {
        uint32_t offset = num * event_size;
        if(offset + 1 >= size) return 0xffff;
        return *(uint16_t*)(start_address + offset);
    }

    virtual unsigned int get_count() {
        return event_count;
    }

    virtual int get_rank(uint16_t score) { 
        uint32_t max_event_count = size / event_size;
        unsigned int num;
        int rank = 1;
        for(num = 0; num < max_event_count; num++) {
            uint16_t s = get_score(num);
            if(s == 0xffff) break;
            if(s <= score) rank++;
        }
        if(event_count < num) event_count = num;
        return rank;
    }

    virtual void write(uint16_t score) {
        uint32_t max_event_count = size / event_size;
        unsigned int num;
        for(num = 0; num < max_event_count; num++) {
            uint16_t s = get_score(num);
            if(s == 0xffff) break;
        }
        if(num == max_event_count) return;
        if(event_count < num) event_count = num;

        uint8_t buffer[256];
        uint32_t index = num * event_size;
        uint32_t blockstart = index & 0xffffff00;

        for(unsigned int i = 0; i < 256; i++) {
            buffer[i] = *(uint8_t *)(start_address + blockstart + i);
        }
        *(uint16_t*)(buffer + (index & 0xff)) = score;

        lidar_stop();
        sleep_ms(500);
        lockout_other_core();
        uint32_t status = save_and_disable_interrupts();
        flash_range_program(intptr_t(start_address + blockstart) - (intptr_t)XIP_BASE, buffer, 256);
        restore_interrupts(status);
        unlockout_other_core();
        event_count++;
        lidar_start();
        //printf("l logger added: index=%d count=%d\n", (int)index, event_count);
    }
};

