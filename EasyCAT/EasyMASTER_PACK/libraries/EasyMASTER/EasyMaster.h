

#ifndef EasyMaster_H
#define EasyMaster_H


#include <Arduino.h> 


//---- EasyMaster library V_0.1 ---------------------------------------------------------------------------------	

void InitMaster(int iCampionatura);
void ExchangeMaster();
void WriteBuffer(int iStart, int iLen, char * pLocalBuffer);
void ReadBuffer(int iStart, int iLen, char * pLocalBuffer);

#endif