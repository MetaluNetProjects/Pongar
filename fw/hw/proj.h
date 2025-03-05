// DMX projector

#pragma once

class DMXProj {
protected:
    int dmx_address = 1;
public:
    virtual void move(float pan, float tilt) = 0;
    virtual void dimmer(uint8_t l) = 0;
    virtual void color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) = 0;
};

class MaxiSpot60: public DMXProj {
public:
    virtual void move(float pan, float tilt);
    virtual void dimmer(uint8_t l);
    virtual void color(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
};

class Movobeam100: public DMXProj {
public:
    virtual void move(float pan, float tilt);
    virtual void dimmer(uint8_t l);
    virtual void color(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
};

class Movobeam100Zoom: public Movobeam100 {
public:
    //virtual void move(float pan, float tilt);
    //virtual void dimmer(uint8_t l);
    virtual void color(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    void zoom(uint8_t z);
};

class ClubSpot: public DMXProj {
public:
    virtual void move(float pan, float tilt);
    virtual void dimmer(uint8_t l);
    virtual void color(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
};

/*typedef ClubSpot PROJ_TYPE;
extern PROJ_TYPE proj_impl;*/
extern DMXProj &proj;

