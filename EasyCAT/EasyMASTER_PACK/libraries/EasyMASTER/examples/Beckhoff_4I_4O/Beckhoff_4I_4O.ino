#include <SPI.h>         // needed for Ethernet library communication with the W5100 (Arduino ver>0018)
#include <Ethernet2.h>
#include <utility/w5500.h>
#include <DueTimer.h>
#include <EasyMaster.h>

unsigned int iMillisec;

char OutputBuffer = 1;
char InputBuffer;

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
  
  iMillisec = millis();
}

void loop()
{  
  if (millis() - iMillisec > 50)
  {
    iMillisec = millis();
    
    OutputBuffer <<= 1;
    OutputBuffer &= 0xFF;
    
    if (OutputBuffer == 16)
    {
      OutputBuffer = 1;
    }
    
    // Beckhoff Output offset 26 (208 / 8)  
    // Beckhoff Input  offset 39 (312 / 8)
 
    ReadBuffer(39, 1, &InputBuffer); 
    
    WriteBuffer(26, 1, &OutputBuffer);   
  }
}
