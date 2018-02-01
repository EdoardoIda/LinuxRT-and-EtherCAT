#include "EasyCAT.h"                
#include <SPI.h>                   

EasyCAT EASYCAT;     

const int onboardLed = 13;
byte masterOnline = 0; // ControlWord: 0 means master is not online, 1 otherwise
byte updateFlag = 0; // When set, the master commanded to set a new frequency, otherwise it hasn't
byte frequency = 0; // The commanded pwm frequency from the master
byte handShaker = 0; // When set, we communicate to the master that the message has been received and processed
byte numberOfCalls = 0; // The number of times the led has been updated since Init
byte cycleCounter = 0; // The number of cycles between each master call to the slave

void setup() {
  pinMode (onboardLed,OUTPUT);
  analogWrite(onboardLed,frequency);
  Serial.begin(115200); 
  if (EASYCAT.Init() == true)                                    
  {                                                               
    Serial.println ("initialized");                            
  }                                                                
  else                                                               
  {                                                              
    Serial.println ("initialization failed");                                                                                          
    pinMode(13, OUTPUT);                                                                                                            
    while(1)                                                      
    {                                                              
      digitalWrite (13, LOW);                                      
      delay(500);                                                   
      digitalWrite (13, HIGH);                                      
      delay(500);                                                  
    }                                                              
  }    
}

void loop() {
  EASYCAT.MainTask();
  Application();  
}

void Application() {  
  masterOnline = EASYCAT.BufferOut.Byte[0];  
  updateFlag = EASYCAT.BufferOut.Byte[1]; 
  frequency = EASYCAT.BufferOut.Byte[2]; 
  if (masterOnline) {                         // If The master is online
    if (updateFlag && !handShaker) { // If the master has requested an update and we haven't already worked that out  
      analogWrite(onboardLed,frequency);  // Assign the updated frequency to Led
      handShaker = 1; //Set the handshaking, we processed the update
      numberOfCalls++; // The master has called one more time    
      cycleCounter = 0;
    } else if (!updateFlag && handShaker) {
      handShaker = 0; //If the master hasn't request an update, it's time to reset the handShaking    
    } else if (handShaker){
      cycleCounter++; // Here we measure the number of cycles incourring between every master call to the slave
                      // The method used is : measure the number of calls between the led update and the reset of the handshaking,
                      // which happens in two consecutive master calls
    } else {
      // Do whatever
    }
  }
  EASYCAT.BufferIn.Byte[0] = handShaker; 
  EASYCAT.BufferIn.Byte[1] = numberOfCalls; 
  EASYCAT.BufferIn.Byte[2] = cycleCounter;
}   

