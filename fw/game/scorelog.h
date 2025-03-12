// scores logging

#pragma once

class Scorelog {
public:
    virtual unsigned int get_count() { return 0; }
    virtual uint16_t get_score(unsigned int num) = 0;
    virtual int get_rank(uint16_t score) = 0;
    virtual void write(uint16_t score) {}
    virtual void clear_all() {}
};

