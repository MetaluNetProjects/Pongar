#pragma once
#include <array>

/*enum class Words {_0 = 0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12,
                  partie = 101, joueur, perdu, gagne, niveau, vies, derniere_vie
                 };*/

enum class Words {
    #define WORDS_MACRO(w, n) w = n,
    #include "words_def.h"
};

constexpr void fill_word_string(const char** word_string) {
    #define WORDS_MACRO(w, n) word_string[n] = #w;
    #include "words_def.h"
}

inline const char *get_word_string(uint8_t n) {
    static bool initialized = false;
    static const char* word_string[256] = {0};
    if(!initialized) {
        fill_word_string(word_string);
        initialized = true;
    }
    return word_string[n];
}


