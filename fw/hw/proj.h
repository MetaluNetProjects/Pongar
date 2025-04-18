// DMX projector

#pragma once

class DMXProj {
protected:
    int dmx_address = 1;
public:
    enum Color { white, red, blue, orange, green, yellow };
    virtual void move(float pan, float tilt) = 0;
    virtual void dimmer(uint8_t l) = 0;
    virtual void gobo(uint8_t g) {}
    virtual void color(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) {}
    virtual void color(Color col) {
        switch(col) {
            case white: color(255, 255, 255); break;
            case red: color(255, 0, 0); break;
            case blue: color(0, 0, 255); break;
            case orange: color(255, 128, 0); break;
            case green: color(0, 255, 0); break;
            case yellow: color(255, 255, 0); break;
        }
    };
    virtual void update() {}
};

class DMXProjCompens: public DMXProj {
protected:
    int target_pan, target_tilt;
    int estimated_pan, estimated_tilt;
    int dmx_pan, dmx_tilt;
    float gain_pan = 0.1, gain_tilt = 0.1;
    bool compens = true;
    virtual void sendmove() = 0;
public:
    virtual void move(float pan, float tilt);
    virtual void update();

    void set_gains(float pan, float tilt) { 
        gain_pan = pan;
        gain_tilt = tilt;
    }
    void enable_compens(bool enable) { 
        compens = enable;
    }
};

class MaxiSpot60: public DMXProjCompens { // un peu trop lent, flou
private:
    virtual void sendmove();
public:
    MaxiSpot60() {
        gain_pan = 0.3;
        gain_tilt = 0.1;
    }
    virtual void dimmer(uint8_t l);
    virtual void color(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    virtual void color(Color col);
    virtual void gobo(uint8_t g);
};

class Movobeam100: public DMXProj { // moteurs ok, mais net impossible
public:
    virtual void move(float pan, float tilt);
    virtual void dimmer(uint8_t l);
    virtual void color(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
};

class Movobeam100Zoom: public Movobeam100 { // n'apporte rien sur le non-zoom
public:
    //virtual void move(float pan, float tilt);
    //virtual void dimmer(uint8_t l);
    virtual void color(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    void zoom(uint8_t z);
};

class ClubSpot: public DMXProj { // figure nette, mais trop lent.
public:
    virtual void move(float pan, float tilt);
    virtual void dimmer(uint8_t l);
    virtual void color(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
};

class EVOBeam: public DMXProj { // figure nette, mais trop lent.
    int target_pan, target_tilt;
    int estimated_pan, estimated_tilt;
    int dmx_pan, dmx_tilt;
    float gain_pan = 0.06, gain_tilt = 0.03;
    void sendmove();
public:
    virtual void move(float pan, float tilt);
    virtual void dimmer(uint8_t l);
    virtual void color(uint8_t r, uint8_t g, uint8_t b, uint8_t w){}
    virtual void color(Color col);
    virtual void update();
    void set_gains(float pan, float tilt) { 
        gain_pan = pan;
        gain_tilt = tilt;
    }
};

extern DMXProj &proj;

