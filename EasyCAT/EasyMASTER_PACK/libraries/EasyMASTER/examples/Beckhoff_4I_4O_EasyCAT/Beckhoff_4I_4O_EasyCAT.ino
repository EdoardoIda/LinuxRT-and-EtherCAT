#include <SPI.h>         // needed for Ethernet library communication with the W5100 (Arduino ver>0018)
#include <Ethernet2.h>
#include <utility/w5500.h>
#include <DueTimer.h>
#include <EasyMaster.h>

unsigned int iMillisec;

char BeckhoffOutputBuffer = 1;
char BeckhoffInputBuffer;

char EasyCATOutputBuffer[32];
char EasyCATInputBuffer[32];

void InterruptCampionatura()
{ 
  ExchangeMaster();
}

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  InitMaster(4000); // 4 milliseconds cycle time
  
  Timer.getAvailable().attachInterrupt(InterruptCampionatura).start(4000);
  
  EasyCATOutputBuffer[0] = 1;
  
  iMillisec = millis();
}

void loop()
{  
  if (millis() - iMillisec > 50)
  {
    iMillisec = millis();
    
    BeckhoffOutputBuffer <<= 1;
    BeckhoffOutputBuffer &= 0xFF;
    
    if (BeckhoffOutputBuffer == 16)
    {
      BeckhoffOutputBuffer = 1;
    }
    
    EasyCATOutputBuffer[0] <<= 1;
    EasyCATOutputBuffer[0] &= 0xFF;
    
    if (EasyCATOutputBuffer[0] == 0)
    {
      EasyCATOutputBuffer[0] = 1;
    }
   
    // Beckhoff Output offset 70 (560 / 8)  
    // Beckhoff Input  offset 83 (664 / 8)

    // EasyCAT Output offset 26 (208 / 8)  
    // EasyCAT Input  offset 26 (208 / 8)

    ReadBuffer(83, 1, &BeckhoffInputBuffer);     
    WriteBuffer(70, 1, &BeckhoffOutputBuffer);  
   
    ReadBuffer(26, 32, EasyCATInputBuffer); 
    WriteBuffer(26, 32, EasyCATOutputBuffer);   
  }
}
