#include <stdio.h>
#include "fraise.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// pico_https
#include "tls_listener.h"
#include "listener.h"
#include "request_handler.h"
#include "pico_nologger.h"
#include <malloc.h>
#include "lwip/timeouts.h"

// pico_ap
extern "C" {
#include "dhcpserver.h"
#include "dnsserver.h"
}

#include "flash_html.h"

#define BOARD pico_w

#if __has_include ("wifi_config.h")
#include "wifi_config.h"
#else
#define WIFI_AP 1
#define WIFI_SSID "picow_test"
#define WIFI_PASS "password"
#endif

unsigned char empty_html_gz[] = {
  0x1f, 0x8b, 0x08, 0x08, 0x66, 0x46, 0xdd, 0x67, 0x02, 0x03, 0x65, 0x6d,
  0x70, 0x74, 0x79, 0x2e, 0x68, 0x74, 0x6d, 0x6c, 0x00, 0xe3, 0xb2, 0xc9,
  0x28, 0xc9, 0xcd, 0xb1, 0xb3, 0x49, 0xca, 0x4f, 0xa9, 0xb4, 0xb3, 0xc9,
  0x30, 0xb4, 0x73, 0xcd, 0x2d, 0x28, 0xa9, 0x54, 0x08, 0xc8, 0x4c, 0xce,
  0x57, 0x08, 0xd7, 0xb3, 0xd1, 0x07, 0x8a, 0xd8, 0xe8, 0x43, 0x24, 0xf5,
  0xc1, 0x2a, 0xb9, 0x00, 0x11, 0xe5, 0x0c, 0xd9, 0x32, 0x00, 0x00, 0x00
};
unsigned int empty_html_gz_len = sizeof(empty_html_gz);

unsigned int html_gz_len = empty_html_gz_len;
unsigned char *html_gz = empty_html_gz;


#define HTML_TABLE_FLASHSIZE (4096 * 8) // 32k
#define HTML_TABLE_START (XIP_BASE + PICO_FLASH_SIZE_BYTES - HTML_TABLE_FLASHSIZE)
FlashHTML flashHTML;

bool cyw43ok = false;
bool connected = false;
int led_ms = 200;

void setled(bool on) {
    if(cyw43ok) cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

int counter = 0;
absolute_time_t count_timeout;

void process_count() {
    if(! time_reached(count_timeout)) return;
    count_timeout = make_timeout_time_ms(led_ms);
    counter++;
    setled(counter % 2);
}

dhcp_server_t dhcp_server;
dns_server_t dns_server;

void setup() {
    mallopt(M_TRIM_THRESHOLD, 1024); // from pico_https hello_world

    flashHTML.init(HTML_TABLE_START, HTML_TABLE_FLASHSIZE);
    int flash_html_len = flashHTML.get_html_size();
    if(flash_html_len != -1) {
        html_gz_len = flashHTML.get_html_size();
        html_gz = flashHTML.html_data_p();
    }

    if (cyw43_arch_init() != 0) {
        printf("cyw43_arch_init failed\n");
        return;
    }
    cyw43ok = true;

    setled(1);
    sleep_ms(500);
    setled(0);

#if WIFI_AP
    cyw43_arch_enable_ap_mode(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK);

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
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return;
    }
    printf("Connected.\n");
#endif

    cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 2000, 1, 1, 1));

    start_logging_server("build/" PICO_BOARD "/docker/hello_world/hello_world_https", PICO_BOARD, "1.0", 15000);
    trace("Starting");
    // Listens on 433 for https
    //TLSListener *tls_listener = new TLSListener();
    //tls_listener->listen(443, RequestHandler::create);

    // Listens on 80 for http
    Listener *tcp_listener = new Listener();
    tcp_listener->listen(80, RequestHandler::create);

    connected = true;
}

void loop() {
    process_count();
    if(cyw43ok) {
        cyw43_arch_lwip_begin();
        cyw43_arch_poll();
        sys_check_timeouts();
        update_watchdog();
        cyw43_arch_lwip_end();
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
        case 111: // get HTML size
            printf("html size: %d\n", (int)flashHTML.get_html_size());
            break;
    }
}

void fraise_receivechars(const char *data, uint8_t len){
	uint8_t command = data[0];
	switch(command) {
	    case 'E': printf("E%s\n", data + 1); break;
	    case 'S': fraise_print_status(); break;
	    case 'l': setled(data[1] != '0'); break;
	    case 'e': // empty page
	        html_gz_len = empty_html_gz_len;
            html_gz = empty_html_gz;
            break;
	    case 'f': // flash_html page
	        html_gz_len = flashHTML.get_html_size();
            html_gz = flashHTML.html_data_p();
            break;
	}
}

