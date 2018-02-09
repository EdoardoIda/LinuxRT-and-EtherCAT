#ifndef EASYCATSLAVE_H
#define EASYCATSLAVE_H

// The slave implementation. It is very similar to what we have on the pc code..

#include <Arduino.h>

#include "EasyCAT.h" 
#include "Led.h"

class EasyCatSlave
{
private:
  constexpr static uint8_t numberOfEasyCatStates = 2;
  // Implementation of the state machine
  typedef void (EasyCatSlave::*StateFunction)();
  enum EasyCatState : uint8_t {
        idle,
        updateSlave,
  };
  enum ToggleState : uint8_t {
        off,
        on,
  };
  enum JobState : uint8_t {
        notDone,
        done,
  };
  // Ethercat utilities
  struct InputPdos {
     uint8_t slaveState = 0;
     uint8_t numberOfCalls = 0;
     uint8_t cycleCounter = 0;
  } inputPdos;
  struct OutputPdos {
     uint8_t slaveStatus = 0;
     uint8_t controlWord = 0;
     uint8_t ledFrequency = 0;
  } outputPdos;

  JobState jobState = notDone;
  ToggleState offlineLedState = off;
  uint32_t controlTime = 0;

  Led *onBoardLed;
  //State machine 
  StateFunction stateMachine[numberOfEasyCatStates] = {&EasyCatSlave::IdleFun,
                                                     &EasyCatSlave::UpdateSlaveFun};
  StateFunction stateManager[numberOfEasyCatStates] = {&EasyCatSlave::IdleTransition,
                                                            &EasyCatSlave::UpdateSlaveTransition};

  void ReadMasterOutputs();
  void WriteMasterInputs();
  void IdleTransition();
  void UpdateSlaveTransition();
  void IdleFun();
  void UpdateSlaveFun();
  void OfflineUpdate();
  void OfflineCheck();
  
public:

  EasyCatSlave(Led *led);
  EasyCAT EASYCAT; // The easycat object
  void StartUp();
  void Application();

};

#endif // EASYCATSLAVE_H

