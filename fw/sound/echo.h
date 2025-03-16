#pragma once

template <unsigned SIZE> class Echo {
private:
    uint16_t length;
    int16_t buffer[SIZE] = {0};
    uint16_t pos;
    uint16_t feedback = 0;
    uint16_t volume = 0;
public:
    Echo(uint16_t _length = SIZE) {
        if(_length > SIZE) length = SIZE;
        else length = _length;
    }
    
    inline int16_t getSample(int32_t insample) {
        int16_t s = buffer[pos];
        int32_t news = (volume * (int32_t)insample + feedback * (int32_t)s) >> 15;
        buffer[pos] = news > 32767 ? 32767 : news < -32767 ? -32767 : news;
        pos++;
        if(pos >= length) pos = 0;
        return s;
    }

    void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(!in_buffer) in_buffer = out_buffer;
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            *out_buffer++ += getSample(*in_buffer++);
        }
    }

    void filter(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(!in_buffer) in_buffer = out_buffer;
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            *out_buffer++ = getSample(*in_buffer++);
        }
    }

    void config(uint16_t _feedback, uint16_t _volume, uint16_t _length = SIZE) {
        feedback = _feedback;
        volume = _volume;
        if(_length > SIZE) length = SIZE;
        else length = _length;
    }
};
