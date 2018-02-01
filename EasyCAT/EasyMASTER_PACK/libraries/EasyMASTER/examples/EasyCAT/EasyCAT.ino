#include <SPI.h>         // needed for Ethernet library communication with the W5100 (Arduino ver>0018)
#include <Ethernet2.h>
#include <utility/w5500.h>
#include <DueTimer.h>
#include <EasyMaster.h>

unsigned int iMillisec;

char OutputBuffer[32];
char InputBuffer[32];

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
  
  OutputBuffer[0] = 1;
  
  iMillisec = millis();
}

void loop()
{  
  if (millis() - iMillisec > 50)
  {
    iMillisec = millis();
    
    OutputBuffer[0] <<= 1;
    OutputBuffer[0] &= 0xFF;
    
    if (OutputBuffer[0] == 0)
    {
      OutputBuffer[0] = 1;
    }
    
    // EasyCAT Output offset 26 (208 / 8)  
    // EasyCAT Input  offset 26 (208 / 8)
 
    ReadBuffer(26, 32, InputBuffer); 
    
    WriteBuffer(26, 32, OutputBuffer);   
  }
}
