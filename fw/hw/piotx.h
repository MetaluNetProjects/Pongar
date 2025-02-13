// PIO TX uart

#pragma once

class PioTx {
private:
    PIO pio;
    uint sm;
    uint pgm_offset;

public:
    void init(int tx_pin_num, int baudrate);
    void putc(char c);
    void print_status();
};

