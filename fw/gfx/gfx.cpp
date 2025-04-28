// led fx
#include "fraise.h"
#include "gfx.h"
#include "pixel.h"
#include "config.h"
#include "math.h"

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

using rgb_t = uint32_t;

static inline int modulo(int a, int b) {
    const int result = a % b;
    return result >= 0 ? result : result + b;
}

class ChaserMode {
public:
    virtual ~ChaserMode() {};
    virtual void update() = 0;
    virtual void init() {};
    inline void set_ring_pixel_rgb(int n, rgb_t rgb) {
        set_ring_pixel(n, rgb >> 16, rgb >> 8, rgb);
    }
    rgb_t rgb_mulf(rgb_t col, float m) {
        uint8_t r = col >> 16, g = col >> 8, b = col;
        return (((uint8_t)CLIP(r * m, 0, 255)) << 16) + (((uint8_t)CLIP(g * m, 0, 255)) << 8) + ((uint8_t)CLIP(b * m, 0, 255));
    }
};

class MocheMode : public ChaserMode {
public:
    virtual void update() {
        static float rot = -1;
        static float speed;
        static uint8_t pixels[NUM_PIXELS][3];
        int ring_leds = config.ring_leds;
        if(rot == -1) {
            for(int i = 0; i < ring_leds; i++) {
                pixels[i][0] = (sin(5.0 * i * 6.28 / ring_leds) + 1.0) * 255.0 * 0.5;
                pixels[i][1] = (sin(3.0 * i * 6.28 / ring_leds) + 1.0) * 255.0 * 0.5;
                pixels[i][2] = (sin(2.0 * i * 6.28 / ring_leds) + 1.0) * 255.0 * 0.5;
            }
        }
        speed += (random() % 2000 - 1000) / (15 * 1000.0);
        speed = CLIP(speed, -2, 2);
        rot += speed;
        if(rot > ring_leds) rot -= ring_leds;
        if(rot < 0) rot += ring_leds;
        for(int i = 0; i < ring_leds; i++) {
            int n = (int(i + rot)) % ring_leds;
            set_ring_pixel(i, pixels[n][0], pixels[n][1], pixels[n][2]);
        }
    }
};

class FlashyMode : public ChaserMode {
private:
    static const int NPOINTS = 7;
    float rot = -1;
    float speed;
    //uint8_t pixels[NUM_PIXELS][3];
    rgb_t pixels[NUM_PIXELS];
    float points[NPOINTS];
    float points_lop[NPOINTS];
    float points_speed[NPOINTS];
    static const rgb_t col_points[NPOINTS];// = {0xff0000, 0xffff00, 0xffffff, 0x00ff00, 0x0000ff, 0x00ffff, 0xff00ff};
    int ring_leds;
public:
    virtual void init() {
        ring_leds = config.ring_leds;
        for(int i = 0; i < NPOINTS; i++) {
            points_lop[i] = points[i] = random() % ring_leds;
            points_speed[i] = 0;
        }

        static rgb_t cols[3] = {0x200000, 0x002000, 0x000020};
        for(int i = 0; i < ring_leds; i++) {
            //pixels[i] = rgb_mulf(col_points[(i * NPOINTS) / ring_leds], 0.15);
            int n = (i * 3 * 2) / ring_leds;
            /*if (n % 2) pixels[i] = 0x3f0000;
            else pixels[i] = 0;*/
            pixels[i] = cols[n % 3];
        }
    }
    virtual void update() {
        speed += (random() % 2000 - 1000) / (35 * 1000.0);
        speed = CLIP(speed, -.2, .2);
        rot += speed;
        if(rot > ring_leds) rot -= ring_leds;
        if(rot < 0) rot += ring_leds;
        for(int i = 0; i < ring_leds; i++) {
            int n = (int(i + rot)) % ring_leds;
            set_ring_pixel_rgb(i, pixels[n]);
        }
        for(int i = 0; i < 5; i++) {
            points_speed[i] += (random() % 2000 - 1000) / (10 * 1000.0);
            points_speed[i] = CLIP(points_speed[i], -1.4, 1.4);
            points[i] += points_speed[i] * points_speed[i] * points_speed[i];
            points_lop[i] += (points[i] - points_lop[i]) * 0.1;
            if(points[i] >= ring_leds) {
                points[i] -= ring_leds;
                points_lop[i] -= ring_leds;
            }
            if(points[i] < 0) {
                points[i] += ring_leds;
                points_lop[i] += ring_leds;
            }
            int incr;
            if(points[i] < points_lop[i]) incr = 1;
            else incr = -1;
            int n = abs(points_lop[i] - points[i]) + 1;
            for(int j = 0; j < n; j++) {
                float l = 1.0 - (float)j / n;
                set_ring_pixel_rgb(modulo(points[i] + j * incr, ring_leds), rgb_mulf(0xffffff, l * l));
            }
        }
    }
};
const rgb_t FlashyMode::col_points[NPOINTS] = {0xff0000, 0xffff00, 0xffffff, 0x00ff00, 0x0000ff, 0x00ffff, 0xff00ff};

class BallsMode : public ChaserMode {
private:
    static const int NPOINTS = 7;
    float points[NPOINTS];
    float points_lop[NPOINTS];
    float points_speed[NPOINTS];
public:
    virtual void init() {
        int ring_leds = config.ring_leds;
        for(int i = 0; i < NPOINTS; i++) {
            points_lop[i] = points[i] = random() % ring_leds;
            points_speed[i] = 0;
        }
    }
    virtual void update() {
        int ring_leds = config.ring_leds;
        for(int i = 0; i < ring_leds; i++) {
            set_ring_pixel_rgb(i, 0);
        }
        for(int i = 0; i < 4; i++) {
            set_spot_pixel(i, 0, 0, 0);
        }
        for(int i = 0; i < 5; i++) {
            points_speed[i] += (random() % 2000 - 1000) / (10 * 1000.0);
            points_speed[i] = CLIP(points_speed[i], -1.4, 1.4);
            points[i] += points_speed[i] * points_speed[i] * points_speed[i];
            points_lop[i] += (points[i] - points_lop[i]) * 0.1;
            if(points[i] >= ring_leds) {
                points[i] -= ring_leds;
                points_lop[i] -= ring_leds;
            }
            if(points[i] < 0) {
                points[i] += ring_leds;
                points_lop[i] += ring_leds;
            }
            int incr;
            if(points[i] < points_lop[i]) incr = 1;
            else incr = -1;
            int n = abs(points_lop[i] - points[i]) + 1;
            for(int j = 0; j < n; j++) {
                float l = 1.0 - (float)j / n;
                set_ring_pixel_rgb(modulo(points[i] + j * incr, ring_leds), rgb_mulf(0xffffff, l * l));
            }
        }
    }
};


Chaser::Chaser() : mode(0), modes{ new BallsMode()} {}

void Chaser::set_mode(int m) {
    mode = m;
    modes[mode]->init();
}

void Chaser::update() {
    modes[mode]->update();
}

