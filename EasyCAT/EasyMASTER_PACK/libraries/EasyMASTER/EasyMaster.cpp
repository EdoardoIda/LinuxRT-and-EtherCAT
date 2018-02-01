
#include "EasyMaster.h"                         
 
#include <SPI.h>         // needed for Ethernet library communication with the W5100 (Arduino ver>0018)
#include <Ethernet2.h>
#include <utility/w5500.h>

#include "Configurazione.h"

void ApriEtherCAT( void );
unsigned char TrasferisciConfigurazioneEtherCAT( unsigned char * pDati, unsigned long dwInputBufferSize  );
unsigned char InizializzazioneEtherCATSchedaRTL8139(  void );

void GestioneRicezioneSchedaRTL8139(  );
void GestioneTrasmissioneSchedaRTL8139( );

void WriteBufferInterna(int iStart, int iLen, char * pLocalBuffer);
void ReadBufferInterna(int iStart, int iLen, char * pLocalBuffer);

extern int		 Ttune;
extern int       iContaCount;
extern int		 iContaMilli;
extern int				iTuttoEtherCAT;
extern int				iEtherCATOperationale;
extern int				iEtherCATConnesso;
extern int				iEtherCATErrore;
extern int				iCollegatoEtherCAT;
extern int				iFaseEtherCAT;
extern int				iCuntaFaseEtherCAT;
extern int				iMinchiaEtherCAT;
extern int				iCuntaScansioneEtherCAT;
extern int				iContaLoopEtherCAT;

extern unsigned short  iContaRxPacchetti;
extern unsigned short  iContaTxPacchetti;
extern int             iContaRinegozia;

extern short        iManda ;
extern short        iRinegozia ;
extern short	    iAggancia;
SOCKET s; // our socket that will be opened in RAW mode

short   iParti;
//long        lCampionaturaMS;
short       lCampionaMilliSec;
   
//---- EasyMaster library V_0.1 ---------------------------------------------------------------------------------	 
	 
void InitMaster(int iCampionatura)
{
// initialize the w5500 chip and open a RAW socket 
  s = 0;
  w5500.init();
  w5500.writeSnMR(s, SnMR::MACRAW); 
  w5500.execCmdSn(s, Sock_OPEN);

  lCampionaMilliSec = iCampionatura / 1000;

  iParti          = 0;
  iEtherCATOperationale = 0;
  iManda  = 0;
  iAggancia = 0;
  iRinegozia = 0;
  Ttune = 0;
  iContaCount=0;
  iContaMilli=0;
  iTuttoEtherCAT=0;
  iEtherCATOperationale=0;;
  iEtherCATConnesso=0;;
  iEtherCATErrore=0;;
  iCollegatoEtherCAT=0;
  iFaseEtherCAT=1;
  iCuntaFaseEtherCAT=0;
  iMinchiaEtherCAT=300;
  iCuntaScansioneEtherCAT=0;
  iContaLoopEtherCAT=0;
  iContaRxPacchetti = 0;
  iContaTxPacchetti = 0;
  iContaRinegozia   = 0;

  ApriEtherCAT( );  
  TrasferisciConfigurazioneEtherCAT( (unsigned char *)Configurazione, iDimensioneConfigurazione  ); 
  InizializzazioneEtherCATSchedaRTL8139();
}

void ExchangeMaster()
{
	if (iTuttoEtherCAT == 70)
	{
		iContaCount++;
		iContaMilli	= iContaCount * lCampionaMilliSec  ;
		
		GestioneRicezioneSchedaRTL8139();	
        
		if (iEtherCATOperationale == 1)
        {
        }                    
		
		GestioneTrasmissioneSchedaRTL8139();
    }
    else
    {
       iParti += lCampionaMilliSec;
       if ( iParti >= 3000 )
       {
           Serial.print("DParti");
           Serial.println(iParti);            
           iTuttoEtherCAT = 70;
           iParti --;
           //iContaTTX = 0;
           //iContaRRX = 0;
           //iCheckTRX = 0;
       }          
	}
}

void WriteBuffer(int iStart, int iLen, char * pLocalBuffer)
{
	WriteBufferInterna(iStart, iLen, pLocalBuffer);
}

void ReadBuffer(int iStart, int iLen, char * pLocalBuffer)
{
	ReadBufferInterna(iStart, iLen, pLocalBuffer);
}

