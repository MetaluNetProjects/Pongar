#pragma once

#include "fraise.h"
#include "fraise_eeprom.h"

class Settings {
private:
    static const int STRLEN = 32;
    // data:
    char ap_ssid[STRLEN];
    char ap_password[STRLEN];
    char sta_ssid[STRLEN];
    char sta_password[STRLEN];
    int16_t tcp_port;
    char hostname[STRLEN];

    void copy_string(char *dest, const char* src, uint8_t len) {
        int l = MIN(len, STRLEN - 1);
        memcpy(dest, src, l);
        dest[l] = 0; // terminate string
    }
    void send_string(const char *prefix, char *str) {
        str[STRLEN - 1] = 0; // make sure the string is terminated
        fraise_printf("%s %s\n", prefix, str);
    }
    void send_param(int which) {
        switch(which) {
            case 1: send_string("ap_ssid", ap_ssid); break;
            case 2: send_string("ap_password", ap_password); break;
            case 3: send_string("sta_ssid", sta_ssid); break;
            case 4: send_string("sta_password", sta_password); break;
            case 5: fraise_printf("tcp_port %d\n", tcp_port); break;
            case 6: send_string("hostname", hostname); break;
        }
    }
    char *get_string(char *str) {
        str[STRLEN - 1] = 0; // make sure the string is terminated
        return str;
    }

public:
    void eeprom_declare() {
        eeprom_declare_data((char*)this, sizeof(Settings));
    }
    void receivebytes(const char* data, uint8_t len) {
        char command = fraise_get_uint8();
        data++; len--;
        switch(command) {
            case 1: copy_string(ap_ssid, data, len); break;
            case 2: copy_string(ap_password, data, len); break;
            case 3: copy_string(sta_ssid, data, len); break;
            case 4: copy_string(sta_password, data, len); break;
            case 5: tcp_port = fraise_get_uint16(); break;
            case 6: copy_string(hostname, data, len); break;

            case 200: send_param(fraise_get_uint8()); break;
            case 255: // save
                if(!strncmp(data, "EEPROM_SAVE", MIN(len, sizeof("EEPROM_SAVE")))) {
                    eeprom_save();
                    fraise_printf("l eeprom saved\n");
                } else {
                    fraise_printf("l eesave error: %*s", len, data);
                }
                break;
        }
        if(command < 200) send_param(command);
    }
    const char *get_ap_ssid() {
        return get_string(ap_ssid);
    }
    const char *get_ap_password() {
        return get_string(ap_password);
    }
    const char *get_sta_ssid() {
        return get_string(sta_ssid);
    }
    const char *get_sta_password() {
        return get_string(sta_password);
    }
    uint16_t get_tcp_port() {
        return tcp_port;
    }
    const char *get_hostname() {
        return get_string(hostname);
    }
};

