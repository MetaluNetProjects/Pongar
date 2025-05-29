// Remote control

#include <map>
#include <string>
#include "fraise.h"
#include "string.h"
#include "game/game.h"
#include "config.h"

#define printf fraise_printf

extern Game game;

static bool is_int(const char *str, int *result)
{
   char * p;
   *result = (int)strtol(str, &p, 10);
   return (*p == 0);
}

static void command_set_state(char * command) {
    char *token = strsep(&command, " ");
    int val;
    if(!is_int(token, &val)) {
        return;
    }
    switch(val) {
        case game.STOP: game.stop(); break;
        case game.STANDBY: game.standby(); break;
        case game.PREPARE: game.prepare(); break;
    }
}

static void command_set_volume(char * command) {
    char *token = strsep(&command, " ");
    int val;
    if(!is_int(token, &val)) return;
    game.set_volume(val);
}

static std::map<std::string, uint8_t*> uint8_map{
    {"projlum", &config.proj_lum},
    {"disStable", &config.disable_wait_stable},
    {"disClose", &config.disable_too_close},
    {"disAlarm", &config.disable_too_close_alarm},
    {"disMany", &config.disable_too_many},
};

static void command_get(const char* command) {
    if(!strcmp(command, "state")) printf("R state %d\n", game.get_state());
    else if(!strcmp(command, "volume")) printf("R volume %d\n", game.get_volume());
    else if(uint8_map.count(command)) {
        printf("R %s %d\n", command, *uint8_map[command]);
    }
}

static void command_set(char* command) {
    char *token = strsep(&command, " ");
    if(!strcmp(token, "state")) command_set_state(command);
    else if(!strcmp(token, "volume")) command_set_volume(command);
    else if(uint8_map.count(token)) {
        char *token2 = strsep(&command, " ");
        int val;
        if(!is_int(token2, &val)) return;
        *uint8_map[token] = val;
    }
    command_get(token);
}

void remote_command(const char* com) {
    char command[256];
    strncpy(command, com, sizeof(command) - 1);
    char* commandp = command;
    char *token = strsep(&commandp, " ");
    if(!strcmp(token, "set")) command_set(commandp);
    else if(!strcmp(token, "get")) command_get(commandp);
}

static void send_game_state() {
    static int last_state =-1;
    int state = game.get_state();
    if(last_state != state) {
        last_state = state;
        command_get("state");
    }
}

void remote_update() {
    send_game_state();
}

