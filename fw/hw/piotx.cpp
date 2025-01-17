// PIO TX uart

#include "fraise.h"
#include "piotx.h"
#include "uart_tx.pio.h"

void PioTx::init(int tx_pin_num, int baudrate) {
    if (!claim_pio_sm_irq(&uart_tx_program, &pio, &sm, &pgm_offset, NULL /*&pio_irq*/)) {
        //panic("failed to setup pio");
        return;
    }
    uart_tx_program_init(pio, sm, pgm_offset, tx_pin_num, baudrate);
}

void PioTx::putc(char c) {
    uart_tx_program_putc(pio, sm, c);
}

void PioTx::print_status() {
    printf("l piotx psol %d %d %d %d\n", PIO_NUM(pio), sm, pgm_offset, uart_tx_program.length);  // pio sm offset length
}

