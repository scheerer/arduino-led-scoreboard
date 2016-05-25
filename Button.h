#ifndef Button_h
#define Button_h

#include "Arduino.h"
#include "Bounce2.h" // Button Debouncer

class Button {
  public:
    Button(uint8_t physicalPin);
    Button(uint8_t physicalPin, uint8_t rfPin);
    void init();
    void update();
    boolean justPressed();
    boolean isHeld();
  private:
    uint8_t _physicalPin, _rfPin;
    Bounce *physicalButton, *rfButton;
    Bounce* createBounceInstance(uint8_t pin, uint8_t mode, uint8_t debounceMs);
};

#endif
