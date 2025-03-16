#pragma once

#include "sound_command.h"

class Patch {
public:
    virtual ~Patch() {}
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) = 0;
    virtual void command(SoundCommand c, int p1, int p2, int p3) = 0;
};
