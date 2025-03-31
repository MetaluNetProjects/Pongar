#include <stdio.h>
#include "fraise.h"
#include "pico/stdlib.h"
#include "lwip/tcp.h"
#include "stdio_tcp.h"
#include "tcp_server.h"

class Pipe {
    private:
        static const int BUFLEN = 2048;
        int in = 0;     // next write
        int out = 0;    // next read
        char buffer[BUFLEN];
    public:
        bool put(char c) {
            int in_tmp = in + 1;
            if(in_tmp == BUFLEN) in_tmp = 0;
            if(in_tmp == out) {
                printf("l pipe overflow!\n");
                return false;
            }
            buffer[in] = c;
            in = in_tmp;
            return true;
        }
        char get() {
            if(out == in) {
                printf("l pipe underflow!\n");
                return 0;
            }
            char c = buffer[out];
            out = out + 1;
            if(out == BUFLEN) out = 0;
            return c;
        }
        bool available() {
            return (out != in);
        }
} pipe;

void *tcp_context = NULL;

void tcp_receive() {
    char buffer[512];
    if(!tcp_server_is_running(tcp_context)) return;
    int rcvd = tcp_server_receive(tcp_context, buffer, sizeof(buffer) - 1);
    if(rcvd) for(int i = 0; i < rcvd; i++) pipe.put(buffer[i]);
}

int stdio_tcp_in_chars(char *buf, int length) {
    tcp_receive();
    int i=0;
    while (i<length && pipe.available()) {
        buf[i++] = pipe.get();
    }
    return i ? i : PICO_ERROR_NO_DATA;
}

void stdio_tcp_out_chars(const char *buf, int length) {
    if(tcp_server_is_running(tcp_context)) {
        tcp_server_send(tcp_context, buf, length);
    }
}

stdio_driver_t stdio_tcp = {
    .out_chars = stdio_tcp_out_chars,
    .in_chars = stdio_tcp_in_chars,
};

stdio_driver_t *stdio_tcp_init(uint16_t port) {
    if(!tcp_context) tcp_context = run_tcp_server(port);
    return &stdio_tcp;
}

