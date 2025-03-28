#pragma once

#include "fraise.h"
#include "http_session.h"

// pico_https
#include "tls_listener.h"
#include "listener.h"
#include "request_handler.h"
#include "pico/cyw43_arch.h"
#include "lwip/timeouts.h"

// pico_ap
extern "C" {
#include "dhcpserver.h"
#include "dnsserver.h"
}

#if __has_include ("wifi_config.h")
#include "wifi_config.h"
#else
//#define WIFI_AP 1
#define WIFI_AP_SSID "picow_test"
#define WIFI_AP_PASS "password"
#define WIFI_STA_SSID "picow_test"
#define WIFI_STA_PASS "password"

#endif

#define printf fraise_printf

class WifiManager {
private:
    int sta_switch_pin;
    bool last_pin;
    absolute_time_t pin_timeout = at_the_end_of_time;
    dhcp_server_t dhcp_server;
    dns_server_t dns_server;
    //Listener tcp_listener;

    enum {NONE, AP, STA} state = NONE;

    void switch_changed() {
        /*switch(state) {
        case NONE: break;
        case AP: 
            RequestHandler::closeAll();
            dns_server_deinit(&dns_server);
            dhcp_server_deinit(&dhcp_server);
            cyw43_arch_disable_ap_mode();
            cyw43_arch_deinit();
            break;
        case STA:
            RequestHandler::closeAll();
            cyw43_arch_disable_sta_mode();
            cyw43_arch_deinit();
            break;
        }*/
        state = NONE;
        if(gpio_get(sta_switch_pin) == 1) { // switch isn't closed: go to AP mode
            printf("l switching to AP mode\n");
            cyw43_arch_enable_ap_mode(WIFI_AP_SSID, WIFI_AP_PASS, CYW43_AUTH_WPA2_AES_PSK);

            ip4_addr_t ip;
            ip4_addr_t mask;
            IP4_ADDR(ip_2_ip4(&ip), 192, 168, 4, 1);
            IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

            // Start the dhcp server
            dhcp_server_init(&dhcp_server, &ip, &mask);
            // Start the dns server
            dns_server_init(&dns_server, &ip);
            state = AP;
        } else { // station mode
            printf("l switching to STATION mode\n");
            cyw43_arch_enable_sta_mode();
            printf("Connecting to Wi-Fi...\n");
            if (cyw43_arch_wifi_connect_timeout_ms(WIFI_STA_SSID, WIFI_STA_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
                printf("failed to connect.\n");
                return;
            }
            printf("Connected.\n");
            state = STA;
        }

        // Now configure the server:
        cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 2000, 1, 1, 1));

        start_logging_server("build/" PICO_BOARD "/docker/hello_world/hello_world_https", PICO_BOARD, "1.0", 15000);
        trace("Starting");
        // Listens on 433 for https
        //TLSListener *tls_listener = new TLSListener();
        //tls_listener->listen(443, RequestHandler::create);

        // Listens on 80 for http
        Listener *tcp_listener = new Listener();
        tcp_listener->listen(80, RequestHandler::create);
    }
public:
    void init(int pin) {
        sta_switch_pin = pin;
        gpio_init(sta_switch_pin);
        gpio_set_dir(sta_switch_pin, GPIO_IN);
        gpio_pull_up(sta_switch_pin);
        sleep_ms(50); // wait for the pin to settle
        last_pin = gpio_get(sta_switch_pin);
        pin_timeout = at_the_end_of_time;
        switch_changed();
    }

    void update() {
        if(last_pin != gpio_get(sta_switch_pin)) {
            last_pin = gpio_get(sta_switch_pin);
            pin_timeout = make_timeout_time_ms(1000);
        }
        if(time_reached(pin_timeout)) {
            pin_timeout = at_the_end_of_time;
            printf("l wifi switch_changed\n");
            sleep_ms(100);
            switch_changed();
            last_pin = gpio_get(sta_switch_pin);
        }

        cyw43_arch_lwip_begin();
        cyw43_arch_poll();
        sys_check_timeouts();
        update_watchdog();
        cyw43_arch_lwip_end();
    }
};

