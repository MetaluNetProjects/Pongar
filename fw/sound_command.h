#pragma once

//enum class SoundCommand {buzz, bounce, tut, ring, note, lfoA, seqplay, seqms, osc1, osc1wf, rev1, seqnew};

enum class SoundCommand {
    #define SFX_MACRO(w, n) w = n,
    #include "sound_command_def.h"
};

constexpr void fill_sound_command_string(const char** command_string) {
    #define SFX_MACRO(w, n) command_string[n] = #w;
    #include "sound_command_def.h"
}

inline const char *get_sound_command_string(uint8_t n) {
    static bool initialized = false;
    static const char* command_string[256] = {0};
    if(!initialized) {
        fill_sound_command_string(command_string);
        initialized = true;
    }
    return command_string[n];
}

