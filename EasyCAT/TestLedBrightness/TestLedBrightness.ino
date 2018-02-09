#include "EasyCatSlave.h"
#include "Led.h"

// Everything is done in the easycat class

#define ledPin 13
Led onBoardLed(ledPin);
EasyCatSlave easyCatSlave(&onBoardLed);

void setup() {
  Serial.begin(115200); 
  easyCatSlave.StartUp(); 
}

void loop() {
  easyCatSlave.Application();  
}

