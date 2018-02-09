#ifndef LED_H
#define LED_H

// Simple led class for this tutorial

#include <Arduino.h>

class Led
{
public:
  Led(int ledPinNumber);
  const int pinNumber;
  void StartUp();
  void ChangeBrightness(char pwmFrequency);
};

#endif // LED_H


