// led fx

#pragma once


class ChaserMode;
class Chaser {
  private:
    int mode = 0;
    ChaserMode *modes[10];
  public:
    Chaser();
    void update();
    void set_mode(int m);
};

