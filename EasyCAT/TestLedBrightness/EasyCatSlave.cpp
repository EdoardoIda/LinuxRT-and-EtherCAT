#include "EasyCatSlave.h"

void EasyCatSlave::ReadMasterOutputs() {
  outputPdos.slaveStatus = EASYCAT.BufferOut.Byte[0];  
  outputPdos.controlWord = EASYCAT.BufferOut.Byte[1]; 
  outputPdos.ledFrequency = EASYCAT.BufferOut.Byte[2]; 
} 

void EasyCatSlave::WriteMasterInputs() {
  EASYCAT.BufferIn.Byte[0] = inputPdos.slaveState; 
  EASYCAT.BufferIn.Byte[1] = inputPdos.numberOfCalls; 
  EASYCAT.BufferIn.Byte[2] = inputPdos.cycleCounter;
}

void EasyCatSlave::IdleTransition()
{
  if(outputPdos.controlWord == updateSlave) {
    /*Debug
    Serial.println("updating");*/
    inputPdos.slaveState = updateSlave;
    jobState = notDone;
  }
}

void EasyCatSlave::UpdateSlaveTransition()
{
  if(outputPdos.controlWord == idle) {
    /*Debug
    Serial.println("idling"); */
    inputPdos.slaveState = idle;
  } 
}

void EasyCatSlave::IdleFun() {}

void EasyCatSlave::UpdateSlaveFun() {
  if (jobState == notDone) {
    /*Debug
    Serial.println("processing..");*/
    onBoardLed->ChangeBrightness(outputPdos.ledFrequency);  // Assign the updated frequency to Led
    inputPdos.numberOfCalls++; // The master has called one more time    
    inputPdos.cycleCounter = 1;
    jobState = done;
  } else {
    /*Debug
    Serial.println("counting..");*/
    /* Here we count each cycle between the master call where the update is required
     * and the consecutive master call, where the handshaking has been processed and 
     * the slave is put to rest again. It was done this way to test the timing of the 
     * easycat library for future use. With an arduino due as processing board, we
     * discovered that the code we are running is executed on average 5 times in 1
     * millisecond.
     */
    inputPdos.cycleCounter++; 
  }
  /*Debug
  Serial.println(inputPdos.cycleCounter);*/
}

void EasyCatSlave::OfflineUpdate() {
  // while the master is offline, show that the slave is operational but not connected
  if (millis()-controlTime>100) { 
    controlTime = millis();
    if (offlineLedState == on) {
      onBoardLed->ChangeBrightness(0); 
      offlineLedState = off;
    } else {
      onBoardLed->ChangeBrightness(255); 
      offlineLedState = on;
    }
  }
}

void EasyCatSlave::OfflineCheck() {
  if (offlineLedState == on) {
      onBoardLed->ChangeBrightness(0); 
      offlineLedState = off;
    }
}

EasyCatSlave::EasyCatSlave(Led *led) :
onBoardLed(led) {
  inputPdos.slaveState = idle;
  }

void EasyCatSlave::StartUp() {
  onBoardLed->StartUp();
  onBoardLed->ChangeBrightness(outputPdos.ledFrequency);
  if (EASYCAT.Init() == true)                                    
  {                                                               
    Serial.println ("initialized");                            
  }                                                                
  else                                                               
  {                                                              
    Serial.println ("initialization failed");                                                                                                                                                                                                   
    while(1)                                                      
    {                                                              
      onBoardLed->ChangeBrightness(0);                                     
      delay(500);                                                   
      onBoardLed->ChangeBrightness(255);                                     
      delay(500);                                                  
    }                                                              
  }    
}

void EasyCatSlave::Application() {
  // This is what is called by the arduino at the maximum speed possible
  EASYCAT.MainTask();
  ReadMasterOutputs();
  if (outputPdos.slaveStatus) {
    OfflineCheck();
    (this->*stateManager[inputPdos.slaveState])();
    (this->*stateMachine[inputPdos.slaveState])();
  } else {
    OfflineUpdate();
  }
  WriteMasterInputs();
}

