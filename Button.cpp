#include "Arduino.h"
#include "Bounce2.h" // Button Debouncer
#include "Button.h"

Button::Button(uint8_t physicalPin) {
  _physicalPin = physicalPin;
}

Button::Button(uint8_t physicalPin, uint8_t rfPin) {
  _physicalPin = physicalPin;
  _rfPin = rfPin;
}

void Button::init() {
  if (_physicalPin) {
    physicalButton = createBounceInstance(_physicalPin, INPUT_PULLUP, 50);
  }
  if (_rfPin) {
    rfButton = createBounceInstance(_rfPin, INPUT, 0);
  }
}

void Button::update() {
  physicalButton->update();
  if (rfButton) {
    rfButton->update();
  }
}

boolean Button::justPressed() {
  return physicalButton->fell() || (rfButton && rfButton->rose());
}

boolean Button::isHeld() {
  return physicalButton->read() == LOW || (rfButton && rfButton->read() == HIGH);
}

Bounce* Button::createBounceInstance(uint8_t pin, uint8_t mode, uint8_t debounceMs) {
  Bounce *bouncer = new Bounce();
  pinMode(pin, mode);
  bouncer->attach(pin);
  bouncer->interval(debounceMs);
  return bouncer;
}

