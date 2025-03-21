/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once
#include "fraise.h"
#include "hardware/flash.h"
#include "pico/sync.h"
#include "stdlib.h"
#include "string.h"

template<class T> class RomTable {
public:
    void init(intptr_t _start_address, uint32_t _item_count) {
        start_address = _start_address;
        item_count = _item_count;
        printf("l reftable init start=%ld count=%ld\n", (uint32_t)start_address, item_count);
        ram_buffer = NULL;
    }

    void copy_flash_to_ram() { // create the ram_buffer and copy flash to it.
        if(is_in_ram()) return;
        ram_buffer = (T *)calloc(item_count, sizeof(T));
        printf("l reftable alloc %ld bytes in ram\n", item_count * sizeof(T));
        for(uint32_t i = 0; i < item_count; i++) {
            memcpy(&ram_buffer[i], &(((T*)start_address)[i]), sizeof(T));
        }
    }

    void save_ram_to_flash() { // save the ram_buffer then free it.
        if(!is_in_ram()) return;
        uint32_t status = save_and_disable_interrupts();
        flash_range_erase(start_address - (intptr_t)XIP_BASE, sizeof(T) * item_count);
        flash_range_program(start_address - (intptr_t)XIP_BASE, (const uint8_t *)ram_buffer, sizeof(T) * item_count);
        restore_interrupts(status);
        discard_ram();
    }

    void discard_ram() { // discard and free the ram_buffer.
        if(!is_in_ram()) return;
        free(ram_buffer);
        ram_buffer = NULL;
    }

    void record_item_to_ram(T &item, uint32_t num_item) {
        if(!is_in_ram()) return;
        memcpy(&ram_buffer[num_item], &item, sizeof(T));
    }

    T &get_item(uint32_t num_item) {
        if(is_in_ram())
            return ram_buffer[num_item];
        else
            return (((T*)start_address)[num_item]);
    }

    inline bool is_in_ram() {
        return ram_buffer != NULL;
    }

    inline uint32_t get_count() {
        return item_count;
    }

private:
    intptr_t start_address;
    uint32_t item_count;
    T *ram_buffer;
};

