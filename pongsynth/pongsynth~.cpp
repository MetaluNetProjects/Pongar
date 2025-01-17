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
#include "config.h"
#include "osc.h"

/* -------------------------- pico sdk compat --------------------------*/
absolute_time_t make_timeout_time_ms(int ms) {
    struct timeval tv;
    struct timeval tv_offset { .tv_sec = 0, .tv_usec = ms * 1000};
    gettimeofday(&tv, 0);
    timeradd(&tv, &tv_offset, &tv);
    return tv;
}

bool time_reached(absolute_time_t t) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return timercmp(&t, &tv, <=);
}

/* -------------------------- pongsynth~ ------------------------------ */
static t_class *pongsynth_class;

typedef struct _pongsynth
{
    t_object x_obj;
    t_float x_f;
    MainPatch *x_patch;
} t_pongsynth;

static void *pongsynth_new(t_floatarg f)
{
    t_pongsynth *x = (t_pongsynth *)pd_new(pongsynth_class);
/*    x->x_f = f;
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
    x->x_phase = 0;
    x->x_conv = 0;
    x->x_band = 22000;*/
    x->x_patch = new MainPatch();
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

static void pongsynth_free(t_pongsynth *x)
{
    delete x->x_patch;
}

#define CLIP(x) (x > 1.0 ? 1.0 : x < -1.0 ? -1.0 : x)
static t_int *pongsynth_perform(t_int *w)
{
    t_pongsynth *x = (t_pongsynth *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    int i;
    int32_t intbuf[AUDIO_SAMPLES_PER_BUFFER] = {0};

    x->x_patch->mix(intbuf, 0);
    for (i = 0; i < n; i++)
        //while (n--)
    {
        *out++ = CLIP(intbuf[i] / 32768.0 /*((random() % 0xFFFF) -32768.0) / 32768.0*/);
    }
    return (w+5);
}

static void pongsynth_dsp(t_pongsynth *x, t_signal **sp)
{
    //x->x_conv = 1./sp[0]->s_sr;
    dsp_add(pongsynth_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void pongsynth_anything(t_pongsynth *x, t_symbol *s, int argc, t_atom *argv)
{
    //post("pongsynth rvc %s", s->s_name);
    MainPatch *p = x->x_patch;
    if(s == gensym("buzz")) { post("buzz!"); p->buzz(); }
    else if(s == gensym("bounce")) p->bounce();
}

#ifdef __cplusplus
extern "C" {
#endif

void pongsynth_tilde_setup(void)
{
    pongsynth_class = class_new(gensym("pongsynth~"), (t_newmethod)pongsynth_new,
        (t_method)pongsynth_free, sizeof(t_pongsynth), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(pongsynth_class, t_pongsynth, x_f);
    class_addmethod(pongsynth_class, (t_method)pongsynth_dsp, gensym("dsp"), A_NULL);
    class_addanything(pongsynth_class, pongsynth_anything);
    Osc::setup();
}

#ifdef __cplusplus
}
#endif

