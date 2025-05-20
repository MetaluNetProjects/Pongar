// audio layer

#include "sound/audiolayer.h"
#include "sound/main_patch.h"

MainPatch main_patch;

AudioLayer::AudioLayer(): patch(main_patch) {
}

void AudioLayer::init(int audio_pin) {}

void AudioLayer::command(SoundCommand c, int p1, int p2, int p3) {
    patch.command(c, p1, p2, p3);
}

void AudioLayer::receivebytes(const char* data, uint8_t len) {}
void AudioLayer::audio_task() {}
void AudioLayer::print_cpu() {}

