/*
Copyright (C) 2025 Antoine Rousseau
all material Copyright (c) 1997-1999 Miller Puckette.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "m_pd.h"
#include "math.h"
#include <stdlib.h>

#include "compat.h"
#include "config.h"
#include "players.h"
#include "game.h"
#include "proj.h"

PongarConfig config;
Movobeam100 proj;
/* -------------------------- pico sdk compat --------------------------*/
absolute_time_t make_timeout_time_ms(int ms) {
    struct timeval tv;
    struct timeval tv_offset { .tv_sec = ms / 1000, .tv_usec = (ms % 1000) * 1000};
    gettimeofday(&tv, 0);
    timeradd(&tv, &tv_offset, &tv);
    return tv;
}

bool time_reached(absolute_time_t t) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return timercmp(&t, &tv, <);
}

absolute_time_t at_the_end_of_time { .tv_sec = (long)1e6};

uint8_t fraise_get_uint8() { return 0;}
/* -------------------------- ponggame ------------------------------ */
static t_class *ponggame_class;

int players_separation = 30;
int players_count = 0;
uint16_t players_pos[PLAYERS_MAX];

typedef struct _ponggame
{
    t_object x_obj;
    t_float x_f;
    t_outlet *x_msgout;
} t_ponggame;

t_ponggame *instance;

static void *ponggame_new(void)
{
    t_ponggame *x = (t_ponggame *)pd_new(ponggame_class);
    x->x_msgout = outlet_new(&x->x_obj, &s_anything);
    instance = x;
    game.init(0, 0);
    return (x);
}

static void ponggame_free(t_ponggame *x)
{
}

static void ponggame_anything(t_ponggame *x, t_symbol *s, int argc, t_atom *argv)
{
    //post("ponggame rvc %s", s->s_name);
    if(s == &s_bang) {
        game.update();
    }
    else if(s == gensym("players")){
        players_count = argc;
        for(int i = 0; i < argc; i++) players_pos[i] = atom_getfloat(&argv[i]);
    }
    else if(s == gensym("prepare")){
        game.prepare();
    }
    else if(s == gensym("start")){
        game.start();
    }
    else if(s == gensym("stop")){
        game.stop();
    }
}

char *SoundCommandString[] = {/*"say", "saypause", "sayclear", */"buzz", "bounce"};

/*char *WordsString[] =  {"zero", "un", "deux", "trois", "quatre", "cinq", "six", "sept", "huit", "neuf", "dix", "onze", "douze",
                  partie = 101, joueur, perdu, gagné};*/

void AudioLayer::command(SoundCommand c, int p1, int p2, int p3) {
    t_atom at[4];
    SETSYMBOL(&at[0], gensym(SoundCommandString[(int)c]));
    SETFLOAT(&at[1], p1);
    SETFLOAT(&at[2], p2);
    SETFLOAT(&at[3], p3);
    outlet_anything(instance->x_msgout, gensym("sound_command"), 4, at);
}

void WavPlayer::play(uint8_t folder, uint8_t track) {
    t_atom at[2];
    SETFLOAT(&at[0], folder);
    SETFLOAT(&at[1], track);
    outlet_anything(instance->x_msgout, gensym("say"), 2, at);
}
void WavPlayer::silence(uint16_t ms){
    t_atom at[1];
    SETFLOAT(&at[0], ms);
    outlet_anything(instance->x_msgout, gensym("saysilence"), 1, at);
}
void WavPlayer::clear(){
    outlet_anything(instance->x_msgout, gensym("sayclear"), 0, NULL);
}

void Movobeam100::move(float pan, float tilt) {
    t_atom at[2];
    SETFLOAT(&at[0], pan);
    SETFLOAT(&at[1], tilt);
    outlet_anything(instance->x_msgout, gensym("proj_goto"), 2, at);
}

void Movobeam100::dimmer(uint8_t l) {
    t_atom at[1];
    SETFLOAT(&at[0], l);
    outlet_anything(instance->x_msgout, gensym("proj_dim"), 1, at);
}

void Movobeam100::color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    t_atom at[4];
    SETFLOAT(&at[0], r);
    SETFLOAT(&at[1], g);
    SETFLOAT(&at[2], b);
    SETFLOAT(&at[3], w);
    outlet_anything(instance->x_msgout, gensym("proj_col"), 4, at);
}

void set_pixel(int n, uint8_t r, uint8_t g, uint8_t b){
    t_atom at[4];
    SETFLOAT(&at[0], n);
    SETFLOAT(&at[1], r);
    SETFLOAT(&at[2], g);
    SETFLOAT(&at[3], b);
    outlet_anything(instance->x_msgout, gensym("pixel"), 4, at);
}

void Players::update() {
    if(steady_count == players_count) {
        steady_timeout = at_the_end_of_time;
        pre_steady_count = -1;
        return;
    }
    if(pre_steady_count != players_count) {
        pre_steady_count = players_count;
        steady_timeout = make_timeout_time_ms(STEADY_MS);
        return;
    }
    if(time_reached(steady_timeout)) {
        steady_count = pre_steady_count;
        steady_timeout = at_the_end_of_time;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

void ponggame_setup(void)
{
    ponggame_class = class_new(gensym("ponggame"), (t_newmethod)ponggame_new,
        (t_method)ponggame_free, sizeof(t_ponggame), 0, A_NULL);
    class_addanything(ponggame_class, ponggame_anything);
}

/*void proj_goto(float pan, float tilt) {
    t_atom at[2];
    SETFLOAT(&at[0], pan);
    SETFLOAT(&at[1], tilt);
    outlet_anything(instance->x_msgout, gensym("proj_goto"), 2, at);
}

void proj_set_light(uint8_t l) {
}*/

#ifdef __cplusplus
}
#endif

