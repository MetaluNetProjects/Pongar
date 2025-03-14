#pragma once
#include <array>

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

