#include <stdio.h>
#include "fraise.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "pico_ws_server/web_socket_server.h"

extern "C" {
#include "dhcpserver.h"
#include "dnsserver.h"
}

#include "flash_html.h"

#define BOARD pico_w

extern unsigned char static_html_gz[];
extern unsigned int static_html_gz_len;
extern unsigned int html_gz_len;
extern unsigned char *html_gz;

unsigned char empty_page[] = "<html><body><h1>Empty Pico W.</h1></body></html>";

unsigned char empty_html_gz[] = {
  0x1f, 0x8b, 0x08, 0x08, 0x66, 0x46, 0xdd, 0x67, 0x02, 0x03, 0x65, 0x6d,
  0x70, 0x74, 0x79, 0x2e, 0x68, 0x74, 0x6d, 0x6c, 0x00, 0xe3, 0xb2, 0xc9,
  0x28, 0xc9, 0xcd, 0xb1, 0xb3, 0x49, 0xca, 0x4f, 0xa9, 0xb4, 0xb3, 0xc9,
  0x30, 0xb4, 0x73, 0xcd, 0x2d, 0x28, 0xa9, 0x54, 0x08, 0xc8, 0x4c, 0xce,
  0x57, 0x08, 0xd7, 0xb3, 0xd1, 0x07, 0x8a, 0xd8, 0xe8, 0x43, 0x24, 0xf5,
  0xc1, 0x2a, 0xb9, 0x00, 0x11, 0xe5, 0x0c, 0xd9, 0x32, 0x00, 0x00, 0x00
};
unsigned int empty_html_gz_len = 72;

#define HTML_TABLE_FLASHSIZE (4096 * 8) // 32k
#define HTML_TABLE_START (XIP_BASE + PICO_FLASH_SIZE_BYTES - HTML_TABLE_FLASHSIZE)
FlashHTML flashHTML;

bool cyw43ok = false;
bool connected = false;

void setled(bool on) {
    if(cyw43ok) cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

#if 1
WebSocketServer wsserver;

void on_connect(WebSocketServer& server, uint32_t conn_id) {
  printf("WebSocket opened\n");
  server.sendMessage(conn_id, "hello web socket!");
}

void on_disconnect(WebSocketServer& server, uint32_t conn_id) {
  printf("WebSocket closed\n");
}

void on_message(WebSocketServer& server, uint32_t conn_id, const void* data, size_t len) {
    //printf("WebSocket message %d\n%s\n", (int)len, (char*)data);
    //printf("%s\n", (int)len, (char*)data);
    char name[128];
    int val;
    int ret = sscanf((const char*)data, "%127s %d", name, &val);
    if(ret == 2) {
        printf("%s : %d (msg:%s)\n", name, val, (const char*)data);
    } else {
        printf("bad conversion: ret=%d msg:%s\n", ret, (const char*)data);
    }
    if(!strcmp(name, "volume")) {
        char buf[128];
        sprintf(buf, "volume=%d", val);
        wsserver.broadcastMessage(buf);
    }
}
#endif

int counter = 0;
absolute_time_t count_timeout;

void process_count() {
    //char buf[256];
    if(! time_reached(count_timeout)) return;
    count_timeout = make_timeout_time_ms(200);
    //sprintf(buf, "counter=%d", counter);
    //wsserver.broadcastMessage(buf);
    counter++;
    setled(counter % 2);
}

dhcp_server_t dhcp_server;
dns_server_t dns_server;

void setup() {
    //stdio_init_all();
    flashHTML.init(HTML_TABLE_START, HTML_TABLE_FLASHSIZE);

    html_gz_len = flashHTML.get_html_size();
    html_gz = flashHTML.html_data_p();

    if (cyw43_arch_init() != 0) {
        printf("cyw43_arch_init failed\n");
        return;
    }
    cyw43ok = true;

    setled(1);
    sleep_ms(1000);
    setled(0);

#if 0
    const char *ap_name = "picow_test";
    const char *password = "password";
    //const char *password = NULL;

    cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t ip;
    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&ip), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

    // Start the dhcp server
    dhcp_server_init(&dhcp_server, &ip, &mask);

    // Start the dns server
    dns_server_init(&dns_server, &ip);
#else
    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms("Oeil", NULL, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return;
    }
    printf("Connected.\n");
#endif

    wsserver.setConnectCallback(on_connect);
    wsserver.setCloseCallback(on_disconnect);
    wsserver.setMessageCallback(on_message);

    bool server_ok = wsserver.startListening(80);
    if (!server_ok) {
        printf("Failed to start WebSocket server\n");
        return;
    }
    printf("WebSocket server started\n");
    connected = true;
}

void loop() {
    process_count();
    if(cyw43ok) cyw43_arch_poll();
    if(connected) {
        wsserver.popMessages();
    }
}

int ledPeriod;
void fraise_receivebytes(const char *data, uint8_t len){
    uint8_t command = fraise_get_uint8();
    switch(command) {
        case 100: { // HTML size
                uint32_t size = fraise_get_uint32();
                flashHTML.set_html_size(size);
                html_gz_len = flashHTML.get_html_size();
            }
            break;
        case 101: { // HTML data
                int start = fraise_get_uint16();
                int count = fraise_get_uint8();
                for(int i = 0; i < count; i++) {
                    flashHTML.set_data(start + i, fraise_get_uint8());
                }
            }
            break;
        case 102: // HTML save
            flashHTML.save_ram_to_flash();
            break;
        case 110: { // read HTML data
                int start = fraise_get_uint16();
                int count = fraise_get_uint8();
                int size = flashHTML.get_html_size();
                fraise_put_init();
                fraise_put_uint8(100);
                uint8_t *html = flashHTML.html_data_p();
                for(int i = 0; i < count; i++) {
                    if(start + i < size) fraise_put_uint8(html[start + i]);
                }
                fraise_put_send();
            }
            break;
    }
}

void fraise_receivechars(const char *data, uint8_t len){
	uint8_t command = data[0];
	switch(command) {
	    case 'E': printf("E%s\n", data + 1); break;
	    case 'S': fraise_print_status(); break;
	    //case 'L': led = (data[1] != '0'); break;
	    case 'l': setled(data[1] != '0'); break;
	    case 'e': // empty page
	        html_gz_len = sizeof(empty_page);
            html_gz = empty_page;
            break;
	    case 'n': // normal page
	        html_gz_len = static_html_gz_len;
            html_gz = static_html_gz;
            break;
	    case 'z': // zipped empty page
	        html_gz_len = empty_html_gz_len;
            html_gz = empty_html_gz;
            break;
	    case 'f': // flash_html page
	        html_gz_len = flashHTML.get_html_size();
            html_gz = flashHTML.html_data_p();
            break;
	}
}

