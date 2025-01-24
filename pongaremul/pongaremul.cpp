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

absolute_time_t at_the_end_of_time { .tv_sec = (long)1e6, .tv_usec = 0};

absolute_time_t get_absolute_time() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv;
}

absolute_time_t boot_time = get_absolute_time();

uint32_t to_ms_since_boot (absolute_time_t t) {
    struct timeval tv;
    timersub(&t, &boot_time, &tv);
    return tv.tv_sec * 1000U + tv.tv_usec / 1000U;
}

uint8_t fraise_get_uint8() { return 0;}
/* -------------------------- pongaremul ------------------------------ */
static t_class *pongaremul_class;

typedef struct _pongaremul
{
    t_object x_obj;
    t_float x_f;
    t_outlet *x_msgout;
    MainPatch *x_patch;
    bool x_wav_is_playing;
} t_pongaremul;

t_pongaremul *instance;

static void *pongaremul_new(void)
{
    t_pongaremul *x = (t_pongaremul *)pd_new(pongaremul_class);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_msgout = outlet_new(&x->x_obj, &s_anything);
    instance = x;
    x->x_wav_is_playing = false;
    game.init(0, 0);
    return (x);
}

static void pongaremul_free(t_pongaremul *x)
{
}

static void pongaremul_anything(t_pongaremul *x, t_symbol *s, int argc, t_atom *argv)
{
    //post("pongaremul rvc %s", s->s_name);
    if(s == &s_bang) {
        game.update();
        game.pixels_update();
    }
    else if(s == gensym("players")){
        /*players_count = argc;
        for(int i = 0; i < argc; i++) players_pos[i] = atom_getfloat(&argv[i]);*/
        int count = argc;
        uint16_t pos[Players::PLAYERS_MAX];
        for(int i = 0; i < argc; i++) pos[i] = atom_getfloat(&argv[i]);
        game.players.set_raw_pos(pos, count);

        t_atom at[2 * Players::PLAYERS_MAX];
        for(int i = 0; i < game.players.get_count(); i++) {
            SETFLOAT(&at[i], game.players.get_pos(i));
            SETFLOAT(&at[i], game.players.get_pos(i));
        }
        outlet_anything(instance->x_msgout, gensym("fplayers"), game.players.get_count(), at);
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
    else if(s == gensym("wav_playing")){
        x->x_wav_is_playing = argc > 0 ? atom_getfloat(&argv[0]) : 0;
    }
    else if(s == gensym("buzz")) x->x_patch->buzz();
    else if(s == gensym("bounce")) x->x_patch->bounce(argc > 0 ? atom_getfloat(&argv[0]) : 0);
}

#define CLIP(x) (x > 1.0 ? 1.0 : x < -1.0 ? -1.0 : x)
static t_int *pongaremul_perform(t_int *w)
{
    t_pongaremul *x = (t_pongaremul *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    int i;
    int32_t intbuf[AUDIO_SAMPLES_PER_BUFFER] = {0};

    x->x_patch->mix(intbuf, 0);
    for (i = 0; i < n; i++)
        //while (n--)
    {
        *out++ = CLIP(intbuf[i] / 32768.0);
        //*out++ = CLIP(((random() % 0xFFFF) -32768.0) / 32768.0);
    }
    return (w+4);
}

static void pongaremul_dsp(t_pongaremul *x, t_signal **sp)
{
    dsp_add(pongaremul_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

/* -------------------------- audio layer ------------------------------ */

char *SoundCommandString[] = {/*"say", "saypause", "sayclear", */"buzz", "bounce"};

/*char *WordsString[] =  {"zero", "un", "deux", "trois", "quatre", "cinq", "six", "sept", "huit", "neuf", "dix", "onze", "douze",
                  partie = 101, joueur, perdu, gagné};*/

void AudioLayer::command(SoundCommand c, int p1, int p2, int p3) {
    /*t_atom at[4];
    SETSYMBOL(&at[0], gensym(SoundCommandString[(int)c]));
    SETFLOAT(&at[1], p1);
    SETFLOAT(&at[2], p2);
    SETFLOAT(&at[3], p3);
    outlet_anything(instance->x_msgout, gensym("sound_command"), 4, at);*/
    main_patch.command(c, p1, p2, p3);
}

void AudioLayer::init(int audio_pin) {
    instance->x_patch = &main_patch;
}

/* -------------------------- wav player ------------------------------ */

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

bool WavPlayer::is_playing(){
    return instance->x_wav_is_playing;
}

/* -------------------------- projector ------------------------------ */

void Movobeam100::move(float pan, float tilt) {
    t_atom at[2];
    SETFLOAT(&at[0], pan);
    SETFLOAT(&at[1], tilt / config.proj_tilt_amp);
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

/*void Players::update() {
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
}*/

/* -------------------------- setup ------------------------------ */

#ifdef __cplusplus
extern "C" {
#endif

void pongaremul_setup(void)
{
    Osc::setup();
    pongaremul_class = class_new(gensym("pongaremul"), (t_newmethod)pongaremul_new,
        (t_method)pongaremul_free, sizeof(t_pongaremul), 0, A_NULL);
    class_addanything(pongaremul_class, pongaremul_anything);
    class_addmethod(pongaremul_class, (t_method)pongaremul_dsp, gensym("dsp"), A_NULL);
}

#ifdef __cplusplus
}
#endif

