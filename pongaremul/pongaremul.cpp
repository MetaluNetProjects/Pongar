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
#include <map>

#include "compat.h"
#include "sound.h"
#include "config.h"
#include "players.h"
#include "game.h"
#include "proj.h"
#include "sound/audiolayer.h"
#include "sound/main_patch.h"

PongarConfig config;

class Logger: public Scorelog{
  public:
    virtual ~Logger() {}
    virtual int get_rank(uint16_t score) { return 111; }
    virtual void write(uint16_t score) {};
    virtual uint16_t get_score(unsigned int num) {return 57; };
};

#define speaker x->x_game->speaker

/* -------------------------- pico sdk compat --------------------------*/
absolute_time_t make_timeout_time_ms(int ms) {
    struct timeval tv;
    struct timeval tv_offset {
        .tv_sec = ms / 1000, .tv_usec = (ms % 1000) * 1000
    };
    gettimeofday(&tv, 0);
    timeradd(&tv, &tv_offset, &tv);
    return tv;
}

bool time_reached(absolute_time_t t) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return timercmp(&t, &tv, < );
}

absolute_time_t at_the_end_of_time { .tv_sec = (long)1e12, .tv_usec = 0};

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

uint8_t fraise_get_uint8() {
    return 0;
}
/* -------------------------- pongaremul ------------------------------ */
static t_class *pongaremul_class;

typedef struct _pongaremul
{
    t_object x_obj;
    t_float x_f;
    t_outlet *x_msgout;
    bool x_wav_is_playing;
    uint16_t x_lidar[360];
    Logger *x_scorelog;
    AudioLayer *x_audio;
    Game *x_game;
    //t_symbol *pix_ring_tabsym;
    //t_symbol *pix_spot_tabsym;
    t_word *ring_vec;
    int ring_npoints;
    t_word *spot_vec;
    int spot_npoints;
} t_pongaremul;

t_pongaremul *instance;

static void *pongaremul_new(void)
{
    t_pongaremul *x = (t_pongaremul *)pd_new(pongaremul_class);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_msgout = outlet_new(&x->x_obj, &s_anything);
    instance = x;
    x->x_wav_is_playing = false;
    x->x_scorelog = new Logger;
    x->x_audio = new AudioLayer;
    x->x_game = new Game(*x->x_scorelog, *x->x_audio);
    x->x_game->init(0);
    x->ring_vec = x->spot_vec = NULL;
    x->ring_npoints = x->spot_npoints = 0;
    return (x);
}

static void pongaremul_free(t_pongaremul *x)
{
    delete x->x_game;
    delete x->x_audio;
    delete x->x_scorelog;
}

static void pongaremul_anything(t_pongaremul *x, t_symbol *s, int argc, t_atom *argv)
{
    //post("pongaremul rvc %s", s->s_name);
    if(s == &s_bang) {
        x->x_game->update();
        x->x_game->pixels_update();
    }
    /*else if(s == gensym("players")) {
        int count = argc;
        Position pos[Players::PLAYERS_MAX];
        for(int i = 0; i < argc; i++) pos[i].angle = atom_getfloat(&argv[i]);
        x->x_game->players.set_raw_pos(pos, count);

        t_atom at[3 * Players::PLAYERS_MAX];
        for(int i = 0; i < x->x_game->players.get_count(); i++) {
            SETFLOAT(&at[3 * i], x->x_game->players.get_pos(i).angle);
            SETFLOAT(&at[3 * i + 1], x->x_game->players.get_pos(i).distance);
            SETFLOAT(&at[3 * i + 2], x->x_game->players.get_pos(i).size);
        }
        outlet_anything(instance->x_msgout, gensym("fplayers"), x->x_game->players.get_count() * 3, at);
    }*/
    else if(s == gensym("prepare")) {
        x->x_game->prepare();
    }
    else if(s == gensym("start")) {
        x->x_game->start();
    }
    else if(s == gensym("stop")) {
        x->x_game->stop();
    }
    else if(s == gensym("standby")) {
        x->x_game->standby();
    }
    else if(s == gensym("wav_playing")) {
        x->x_wav_is_playing = argc > 0 ? atom_getfloat(&argv[0]) : 0;
    }
    else if(s == gensym("sfx")) {
        static std::map<t_symbol*, int> commands;
        if(commands.empty()) {
            for(int i = 0; i < 256; i++) {
                const char *w = get_sound_command_string(i);
                if(w) commands[gensym(w)] = i;
            }
        }
        int com = 0;
        if(argc > 0) {
            if((&argv[0])->a_type == A_SYMBOL) {
                t_symbol *sym = atom_getsymbol(&argv[0]);
                if(commands.count(sym) == 0) {
                    pd_error(x, "%s: no such sfx command!", sym->s_name);
                    return;
                }
                com = commands[atom_getsymbol(&argv[0])];
            }
            else com = atom_getfloat(&argv[0]);
        }
        int p1 = argc > 1 ? atom_getfloat(&argv[1]) : 0;
        int p2 = argc > 2 ? atom_getfloat(&argv[2]) : 0;
        int p3 = argc > 3 ? atom_getfloat(&argv[3]) : 0;
        x->x_audio->command((SoundCommand)com, p1, p2, p3);
    }
    else if(s == gensym("lidar")) {
        t_symbol *tabname = atom_getsymbol(&argv[0]);
        t_garray *a;
        int npoints;
        t_word *vec;

        if (!(a = (t_garray *)pd_findbyclass(tabname, garray_class)))
            pd_error(x, "%s: no such array", tabname->s_name);
        else if (!garray_getfloatwords(a, &npoints, &vec))
            pd_error(x, "%s: bad template for tabread", tabname->s_name);
        else
        {
            for(int i = 0; i < 360 && i < npoints; i++) x->x_lidar[i] = vec[i].w_float;
            //x->x_game->players.find_players(x->x_lidar);
            x->x_game->players.find_players_light(x->x_lidar);

            /*t_atom at[4 * Players::PLAYERS_MAX];
            if(x->x_game->players.get_object_set().size() >= Players::PLAYERS_MAX) {
                pd_error(x, "too many objects!");
            } else {
                int n = 0;
                //printf("nb objs:%ld\n", x->x_game->players.get_object_set().size());
                for(int i : x->x_game->players.get_object_set()) {
                    Position &p = x->x_game->players.get_object_pos(i);
                    if(&p != &null_position) {
                        //printf("obj %d angle:%d\n", i, p.angle);
                        SETFLOAT(&at[n], i);
                        n++;
                        SETFLOAT(&at[n], p.angle);
                        n++;
                        SETFLOAT(&at[n], p.distance);
                        n++;
                        SETFLOAT(&at[n], p.size);
                        n++;
                    }
                }
                //printf("nbatoms: %d\n", n);
                outlet_anything(instance->x_msgout, gensym("fobjects"), n, at);
            }

            if(x->x_game->players.get_set().size() >= Players::PLAYERS_MAX) {
                pd_error(x, "too many players!");
            } else {
                int n = 0;
                for(int i : x->x_game->players.get_set()) {
                    Position &p = x->x_game->players.get_pos(i);
                    if(&p != &null_position) {
                        SETFLOAT(&at[n], i);
                        n++;
                        SETFLOAT(&at[n], p.angle);
                        n++;
                        SETFLOAT(&at[n], p.distance);
                        n++;
                        SETFLOAT(&at[n], p.size);
                        n++;
                    }
                }
                outlet_anything(instance->x_msgout, gensym("fplayers"), n, at);
            }
            */
        }
    }
    else if(s == gensym("testrand")) {
        int mod = atom_getfloat(&argv[0]);
        int count[100] = {0};
        const int total = 10000000;
        if(mod < 2 || mod > 100) return;
        for(int i = 0; i < total; i++) {
            int r = random() % mod;
            count[r]++;
        }
        for(int i = 0; i < mod; i++) printf("%d: %f\n", i, ((float)mod) * count[i] / (float)total - 1.0);
    }
    else if(s == gensym("testwords")) {
        int n = atom_getfloat(&argv[0]);
        printf("words %d: %s\n", n , get_word_string(n));
    }
    else if (s == gensym("get_words")) {
        t_atom at[2];
        for(int i = 0; i < 256; i++) {
            const char *w = get_word_string(i);
            if(!w) w = "_none_";
            //printf("word %d %s\n", i , w);
            SETFLOAT(&at[0], i);
            SETSYMBOL(&at[1], gensym(w));
            outlet_anything(instance->x_msgout, gensym("word"), 2, at);
        }
    }
    else if (s == gensym("reload_words_duration")) {
        speaker.reload_durations();
    }
    else if (s == gensym("saynumber")) {
        int n = atom_getfloat(&argv[0]);
        speaker.saynumber(n);
    }
    else if (s == gensym("say_hundredths")) {
        float f = atom_getfloat(&argv[0]);
        speaker.say_hundredths(f);
    }
    else if (s == gensym("say_time")) {
        float f = atom_getfloat(&argv[0]);
        speaker.say_time(f);
    }
    else if(s == gensym("make_piece")) {
        if(argc < 3) return;
        float scale = atom_getfloat(&argv[0]);
        int steps = atom_getfloat(&argv[1]);
        int nchords = argc - 2;
        Piece &piece = ((MainPatch&)x->x_audio->get_patch()).seq.piece;
        piece.make(scale);
        Piece::plan_t plan;
        for(int i = 0; i < nchords; i++) plan.push_back(atom_getfloat(&argv[i + 2]));
        piece.set_plan(steps, plan);
        printf("piece steps %d\n", piece.get_total_steps());
        ((MainPatch&)x->x_audio->get_patch()).seq.set_playing(true, true);
    }
    else if(s == gensym("play_drums")) {
        float play_drums = atom_getfloat(&argv[0]) != 0;
        ((MainPatch&)x->x_audio->get_patch()).seq.set_play_drums(play_drums);
    }
    else if(s == gensym("sustain_ms")) {
        int ms = atom_getfloat(&argv[0]);
        ((MainPatch&)x->x_audio->get_patch()).seq.piece.set_sustain_ms(ms);
    }
    else if(s == gensym("play_happy")) {
        ((MainPatch&)x->x_audio->get_patch()).seq.play_happy();
    }
    else if(s == gensym("play_sad")) {
        ((MainPatch&)x->x_audio->get_patch()).seq.play_sad();
    }
    else if(s == gensym("wf_lfo_porta")) {
        Piece &piece = ((MainPatch&)x->x_audio->get_patch()).seq.piece;
        int wf = atom_getfloat(&argv[0]);
        float lfo_f = atom_getfloat(&argv[1]);
        float lfo_a = atom_getfloat(&argv[2]);
        float porta_ms = atom_getfloat(&argv[3]);
        for(int i = 0; i < 4; i++)
            piece.get_voice(i)->set_wf_lfo_porta(wf, lfo_f, lfo_a, porta_ms);
    }
    else if(s == gensym("filter")) {
        Piece &piece = ((MainPatch&)x->x_audio->get_patch()).seq.piece;
        int f = atom_getfloat(&argv[0]);
        int f_rnd = atom_getfloat(&argv[1]);
        float q = atom_getfloat(&argv[2]);
        int porta_ms = atom_getfloat(&argv[3]);
        for(int i = 0; i < 4; i++)
            piece.get_voice(i)->set_filter(f, f_rnd, q, porta_ms);
    }
    else if(s == gensym("asr")) {
        Piece &piece = ((MainPatch&)x->x_audio->get_patch()).seq.piece;
        int attack = atom_getfloat(&argv[0]);
        int sustain = atom_getfloat(&argv[1]);
        int release = atom_getfloat(&argv[2]);
        for(int i = 0; i < 4; i++)
            piece.get_voice(i)->set_asr_ms(attack, sustain, release);
    }
    else if(s == gensym("pixtables")) {
        x->ring_vec = x->spot_vec = NULL;
        x->ring_npoints = x->spot_npoints = 0;
        t_symbol *tabname = atom_getsymbol(&argv[0]);
        t_garray *a;
        int npoints;
        if (!(a = (t_garray *)pd_findbyclass(tabname, garray_class))) {
            pd_error(x, "%s: no such array", tabname->s_name);
            return;
        }
        else if (!garray_getfloatwords(a, &npoints, &x->ring_vec)) {
            pd_error(x, "%s: bad template for ring table", tabname->s_name);
            return;
        }
        x->ring_npoints = npoints / 3;
        tabname = atom_getsymbol(&argv[1]);
        if (!(a = (t_garray *)pd_findbyclass(tabname, garray_class))) {
            pd_error(x, "%s: no such array", tabname->s_name);
            return;
        }
        else if (!garray_getfloatwords(a, &npoints, &x->spot_vec)) {
            pd_error(x, "%s: bad template for ring table", tabname->s_name);
            return;
        }
        x->spot_npoints = npoints / 3;
        printf("ring_npoints %d spot_npoints %d\n", x->ring_npoints, x->spot_npoints);
        //x->pix_ring_tabsym = atom_getsymbol(&argv[0]);
        //x->pix_spot_tabsym = atom_getsymbol(&argv[1]);
    }
}

static t_int *pongaremul_perform(t_int *w)
{
    t_pongaremul *x = (t_pongaremul *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    int i;
    int32_t intbuf[AUDIO_SAMPLES_PER_BUFFER] = {0};

    x->x_audio->mix(intbuf, 0);
    for (i = 0; i < n; i++)
    {
        *out++ = CLIP(intbuf[i] / 32768.0, -1.0, 1.0);
    }
    return (w + 4);
}

static void pongaremul_dsp(t_pongaremul *x, t_signal **sp)
{
    dsp_add(pongaremul_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

/* -------------------------- wav player ------------------------------ */

void WavPlayer::play(uint8_t folder, uint8_t track) {
    t_atom at[2];
    SETFLOAT(&at[0], folder);
    SETFLOAT(&at[1], track);
    outlet_anything(instance->x_msgout, gensym("say"), 2, at);
}
void WavPlayer::silence(uint16_t ms) {
    t_atom at[1];
    SETFLOAT(&at[0], ms);
    outlet_anything(instance->x_msgout, gensym("saysilence"), 1, at);
}
void WavPlayer::clear() {
    outlet_anything(instance->x_msgout, gensym("sayclear"), 0, NULL);
}

bool WavPlayer::is_playing() {
    return instance->x_wav_is_playing;
}

/* -------------------------- projector ------------------------------ */

class FakeProj: public DMXProj {
public:
    virtual void move(float pan, float tilt);
    virtual void dimmer(uint8_t l);
    virtual void color(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
} _proj;
DMXProj &proj = _proj;

void FakeProj::move(float pan, float tilt) {
    t_atom at[2];
    SETFLOAT(&at[0], pan);
    SETFLOAT(&at[1], tilt / config.proj_tilt_amp);
    outlet_anything(instance->x_msgout, gensym("proj_goto"), 2, at);
}

void FakeProj::dimmer(uint8_t l) {
    t_atom at[1];
    SETFLOAT(&at[0], l);
    outlet_anything(instance->x_msgout, gensym("proj_dim"), 1, at);
}

void FakeProj::color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    t_atom at[4];
    SETFLOAT(&at[0], r);
    SETFLOAT(&at[1], g);
    SETFLOAT(&at[2], b);
    SETFLOAT(&at[3], w);
    outlet_anything(instance->x_msgout, gensym("proj_col"), 4, at);
}

/* -------------------------- pixels ------------------------------ */

inline void set_pixel(t_word *vec, int npoints, int n, uint8_t r, uint8_t g, uint8_t b) {
    if(n >= 0 && n < npoints) {
        vec[n * 3 + 0].w_float = r;
        vec[n * 3 + 1].w_float = g;
        vec[n * 3 + 2].w_float = b;
    }
}

void set_ring_pixel(int n, uint8_t r, uint8_t g, uint8_t b) {
    if((instance->ring_vec == NULL) || (instance->ring_npoints == 0)) return;
    set_pixel(instance->ring_vec, instance->ring_npoints, n, r, g, b);
    /*t_atom at[4];
    SETFLOAT(&at[0], n);
    SETFLOAT(&at[1], r);
    SETFLOAT(&at[2], g);
    SETFLOAT(&at[3], b);
    outlet_anything(instance->x_msgout, gensym("pixel"), 4, at);*/
}

void set_spot_pixel(int n, uint8_t r, uint8_t g, uint8_t b) {
    if((instance->spot_vec == NULL) || (instance->spot_npoints == 0)) return;
    set_pixel(instance->spot_vec, instance->spot_npoints, n, r, g, b);
    /*t_atom at[4];
    SETFLOAT(&at[0], n);
    SETFLOAT(&at[1], r);
    SETFLOAT(&at[2], g);
    SETFLOAT(&at[3], b);
    outlet_anything(instance->x_msgout, gensym("spot"), 4, at);*/
}

inline uint8_t blend_color(uint8_t col, uint8_t prev_col, uint8_t alpha) {
    return ((int)col * alpha + (int)prev_col * (255 - alpha)) / 255;
}

inline void get_pixel(t_word *vec, int npoints, int n, uint8_t &r, uint8_t &g, uint8_t &b) {
    n = MIN(npoints, MAX(0, n));
    r = vec[n * 3 + 0].w_float;
    g = vec[n * 3 + 1].w_float;
    b = vec[n * 3 + 2].w_float;
}

inline void blend_pixel(t_word *vec, int npoints, int n, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha) {
    uint8_t prev_r, prev_g, prev_b;
    get_pixel(vec, npoints, n, prev_r, prev_g, prev_b);
    set_pixel(vec, npoints, n, blend_color(r, prev_r, alpha), blend_color(g, prev_g, alpha), blend_color(b, prev_b, alpha));
}

void blend_ring_pixel(int n, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha) {
    if((instance->ring_vec == NULL) || (instance->ring_npoints == 0)) return;
    blend_pixel(instance->ring_vec, instance->ring_npoints, n, r, g, b, alpha);
}

void blend_spot_pixel(int n, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha) {
    if((instance->spot_vec == NULL) || (instance->spot_npoints == 0)) return;
    blend_pixel(instance->spot_vec, instance->spot_npoints, n, r, g, b, alpha);
}

/* -------------------------- setup ------------------------------ */

#ifdef __cplusplus
extern "C" {
#endif

void pongaremul_setup(void)
{
    pongaremul_class = class_new(gensym("pongaremul"), (t_newmethod)pongaremul_new,
                                 (t_method)pongaremul_free, sizeof(t_pongaremul), 0, A_NULL);
    class_addanything(pongaremul_class, pongaremul_anything);
    class_addmethod(pongaremul_class, (t_method)pongaremul_dsp, gensym("dsp"), A_NULL);
}

#ifdef __cplusplus
}
#endif

