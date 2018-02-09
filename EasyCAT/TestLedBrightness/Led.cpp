#include "Led.h"

Led::Led(int ledPinNumber) : 
pinNumber(ledPinNumber) {}

void Led::StartUp() {
  pinMode (pinNumber,OUTPUT);
}

void Led::ChangeBrightness(char pwmFrequency) {
   analogWrite(pinNumber,pwmFrequency);
   /* Debug
   Serial.print("Led Frequency = ");
   Serial.println(int(pwmFrequency));
   */
}

