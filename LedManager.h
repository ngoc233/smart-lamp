#ifndef setupLed_h
#include "Arduino.h"
#define setupLed_h

class LedManager{
  public:
    LedManager();
    // setup pinmod
    void SETUP();

    //turn on digital led
    void ONDILED(int gpio);

    //turn off digital led
    void OFFDILED(int gpio);

    //turn on analog led
    void ONANLED(int gpio, int value);

    //turn on analog led
    void OFFANLED(int gpio);

    //fade mode
    void FADE(int red,int green,int blue,int white);

    //blink mode
    void BLINK(int red,int green,int blue,int white);
};

extern LedManager ledManager;
// gpio address for rgb
extern int WHITE_PIN;
extern int RED_PIN;
extern int GREEN_PIN;
extern int BLUE_PIN;
#endif
