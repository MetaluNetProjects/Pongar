#pragma once

#include "romtable.h"

class FlashHTML: public RomTable<uint8_t> {
public:
    uint32_t *html_size_p() {
        return (uint32_t*)&get_item(0);
    }

    uint32_t get_html_size() {
        return *html_size_p();
    }

    void set_html_size(uint32_t size) {
        if(!is_in_ram()) copy_flash_to_ram();
        *html_size_p() = size;
    }

    uint8_t *html_data_p() {
        return &get_item(4);
    }

    void set_data(int index, uint8_t data) {
        if(!is_in_ram()) copy_flash_to_ram();
        *(html_data_p() + index) = data;
    }
};
