#include <SPI.h>         // needed for Ethernet library communication with the W5100 (Arduino ver>0018)
#include <Ethernet2.h>
#include <utility/w5500.h>

#include "AxesBrainEcInterface.h"
#include "Strutture.h"
#include "AxesBrainEcNpfDevice.h"
#include "AxesBrainEcDevice.h"
extern short       lCampionaMilliSec;

extern SOCKET s;    // our socket that will be opened in RAW mode

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////             EcDevice            ///////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


#define   RINEGOZIA             3000
#define   MAX_PACCHETTI         3

unsigned short  iContaRxPacchetti;
unsigned short  iContaTxPacchetti;
int             iContaRinegozia;

unsigned char   cPacchettoRX[MAX_PACCHETTI][ETHERNET_MAX_FRAMEBUF_LEN];
unsigned char   cPacchettoTX[MAX_PACCHETTI][ETHERNET_MAX_FRAMEBUF_LEN];


unsigned char	SocketBuffer[ETHERNET_MAX_FRAMEBUF_LEN];

extern int           iCollegatoEtherCAT;
extern short          iManda ;
extern short          iRinegozia ;
extern int           iTuttoEtherCAT;
extern PEcDevice     pEcDevice     ;

void MandaDoppioPacchetto( void * pData, unsigned int iLung );

/*long max (long a, long b )
{
	if (a>b)
	{
		return a;
	}
	else
	{
		return b;
	}
}


long min (long a, long b )
{
	if (a>b)
	{
		return b;
	}
	else
	{
		return a;
	}
}*/



unsigned short  minS (unsigned short  a, unsigned short  b )
{
	if (a>b)
	{
		return b;
	}
	else
	{
		return a;
	}
}

unsigned short  maxS (unsigned short   a, unsigned short   b )
{
	if (a>b)
	{
		return a;
	}
	else
	{
		return b;
	}
}





///\brief Frees a previously allocated Ethernet frame.
void FrameFreeEcDevice( PEcDevice  pEcDevice, void * pData)
{ 
}
	
///\brief Allocates memory for an Ethernet frame.
void * FrameAllocEcDevice( PEcDevice  pEcDevice, unsigned long nData)
{ 
	{
		void * pPippo = &cPacchettoTX[iContaTxPacchetti];
		
		iContaTxPacchetti++;
		if ( iContaTxPacchetti >= MAX_PACCHETTI)
		{
			iContaTxPacchetti = 0;
		}  

		return pPippo;
	}
}


///\brief Sends a frame to the network adapter and frees it afterwards.
void FrameSendAndFreeEcDevice( PEcDevice  pEcDevice, void * pData, unsigned long  nData)
{ 
	MandaDoppioPacchetto( pData, nData );
	FrameFreeEcDevice(pEcDevice, pData); 
}
	

unsigned long GetOutputSizeEcDevice( PEcDevice  pEcDevice )
{
	return pEcDevice->m_pImage->m_nOut;
}


unsigned long GetInputSizeEcDevice( PEcDevice  pEcDevice )
{
	return pEcDevice->m_pImage->m_nIn;
}

unsigned char * GetInputPtrEcDevice( PEcDevice  pEcDevice, unsigned long offs, unsigned long size )
{
	if ( pEcDevice->m_pImage->m_pIn && (offs + size <= pEcDevice->m_pImage->m_nIn) )
		return &pEcDevice->m_pImage->m_pIn[offs];
	else
		return NULL;
}


unsigned char * GetOutputPtrEcDevice( PEcDevice  pEcDevice, unsigned long offs, unsigned long size )
{
	if ( pEcDevice->m_pImage->m_pOut && (offs + size <= pEcDevice->m_pImage->m_nOut) )
		return &pEcDevice->m_pImage->m_pOut[offs];
	else
		return NULL;
}


unsigned int OpenEcDevice(PEcDevice pEcDevice)
{
	if ( !pEcDevice->m_pImage )
	{
		return EC_E_OPENFAILED;
	}
	
	if( GetOutputSizeEcDevice( pEcDevice ) < sizeof(unsigned short) || GetInputSizeEcDevice(pEcDevice) < sizeof(unsigned short) )
	{
		return EC_E_OPENFAILED;
	}	
		
	//get pointer to the device state word
	pEcDevice->m_pDevState	= ( unsigned short * )GetOutputPtrEcDevice( pEcDevice, GetOutputSizeEcDevice(pEcDevice) - sizeof(unsigned short), sizeof(unsigned short) );
	//get pointer to the device control word
	pEcDevice->m_pDevCtrl	= ( unsigned short * )GetInputPtrEcDevice( pEcDevice, GetInputSizeEcDevice(pEcDevice) - sizeof(unsigned short), sizeof(unsigned short) );
	
	if ( pEcDevice->m_pDevState == NULL || pEcDevice->m_pDevCtrl == NULL )
	{
		return EC_E_OPENFAILED;
	}
	
	SetResetMask(*pEcDevice->m_pDevState, ETHERNETRTMP_STATE_NOMINIPORT, 0);
	
	if( pEcDevice->m_pEcMaster )
	{
		//Set MAC address of the master.		
		SetSrcMacAddressEcMaster( pEcDevice->m_pEcMaster, pEcDevice->m_macAdapter);		
		
		//Set master state to Init
		pEcDevice->m_pEcMaster->m_reqState = DEVICE_STATE_INIT;
		
		//initialize master
		int hr = OpenEcMaster( pEcDevice->m_pEcMaster );
		if ( hr )
			return hr;		
	}	

	pEcDevice->m_bOpened = 1;
	

	return 1;
}


unsigned int CloseEcDevice(PEcDevice pEcDevice)
{
	pEcDevice->m_bOpened = 0;
	if( pEcDevice->m_pEcMaster )
	{
		pEcDevice->m_pEcMaster->m_currState = EC_MASTER_STATE_INIT;
	}
	return 1;
}




//////////////////////////////////////////////////////////////////////	
///\brief Sends the queued EtherCAT frames to the network adapter.
void SendQueuedE88A4CmdsEcDevice(PEcDevice pEcDevice, long maxTime)
{
	if ( pEcDevice->m_pEcMaster )
	{
		PETHERNET_88A4_FRAME pFrame;
		while ( pEcDevice->m_pFifoE88A4->Remove(pFrame) )
		{
			unsigned long pino;
                        pino = SIZEOF_88A4_FRAME(pFrame);
			FrameSendAndFreeEcDevice(pEcDevice, pFrame, pino);							
		}
	}

	return ;
}

///////////////////////////////////////////////////////////////////////////////
void	SendQueuedEcCmdsEcDevice( PEcDevice pEcDevice,long maxTime)
{


	if ( pEcDevice->m_pEcMaster != NULL )
	{
		if ( pEcDevice->m_pActiveEcFrame != NULL )
		{
			if ( pEcDevice->m_pActiveEcFrame->timeout > 0 )
			{
				pEcDevice->m_pActiveEcFrame->timeout--;
			}
			else
			{
				pEcDevice->m_pActiveEcFrame->result		= ECERR_DEVICE_TIMEOUT;
				pEcDevice->m_pActiveEcFrame->received	= 1;
			}
		}
		else if ( pEcDevice->m_pFifoEcFrame->Remove(pEcDevice->m_pActiveEcFrame) )
		{	// maximum 1 frame per cycle
			unsigned long size = pEcDevice->m_pActiveEcFrame->cbLength + ETHERNET_88A4_FRAME_LEN;
			PETHERNET_88A4_FRAME pSend = (PETHERNET_88A4_FRAME)FrameAllocEcDevice(pEcDevice, size);
			if ( pSend )
			{
				pSend->Ether.Destination	= BroadcastEthernetAddress;
				pSend->Ether.Source			= pEcDevice->m_macAdapter;
				pSend->Ether.FrameType		= ETHERNET_FRAME_TYPE_BKHF_SW;
				pSend->E88A4.Type			= ETYPE_88A4_TYPE_ECAT;
				pSend->E88A4.Length			= pEcDevice->m_pActiveEcFrame->cbLength;
				pSend->E88A4.Reserved		= 0;
				memcpy(ENDOF(pSend), ENDOF(pEcDevice->m_pActiveEcFrame), pEcDevice->m_pActiveEcFrame->cbLength);
				FrameSendAndFreeEcDevice( pEcDevice, pSend, size);

	
			}
		}
	}

	return ;
}

unsigned short CheckFrameEcDevice( PEcDevice pEcDevice, void * pData, unsigned long nData, int &bPending)
{
	if ( nData < ETHERNET_FRAME_LEN )
		return 0;

	PETHERNET_FRAME			  pFrame		= (PETHERNET_FRAME)pData;
	unsigned short *		  pFrameType	= FRAMETYPE_PTR(pFrame);
	

	bPending = 0;
	switch ( *pFrameType )
	{
	case ETHERNET_FRAME_TYPE_BKHF_SW:
		{	//E88A4	Frame type		
			PETHERNET_88A4_MAX_HEADER	p88A4	= (PETHERNET_88A4_MAX_HEADER)ENDOF(pFrameType);
			if ( p88A4->E88A4.Type == ETYPE_88A4_TYPE_ECAT )
			{	//EtherCAT frame
				unsigned long e88A4Len = p88A4->E88A4.Length;
				PETYPE_EC_HEADER pHead = &p88A4->FirstEcHead;				

				iCollegatoEtherCAT			= 0;


				if ( pHead->idx != EC_HEAD_IDX_EXTERN_VALUE )
				{
					int iSuc;
					// EcMaster
					if( pEcDevice->m_pEcMaster )
					{
						iSuc = CheckFrameEcMaster(pEcDevice->m_pEcMaster, pFrame, nData, p88A4, bPending);						
						
						if (iSuc == 1 )
						{
							return 1;
						}
					}
				}	
				else
				{
					if ( pEcDevice->m_pActiveEcFrame && pEcDevice->m_pActiveEcFrame->received == 0 )
					{	// m_pActiveEcFrame comes from an AdsReadWriteInd or AdsWriteInd
						if ( EcCmdHash( &p88A4->FirstEcHead) == pEcDevice->m_pActiveEcFrame->ecCmdHash )
						{	// returning frame
							if ( p88A4->E88A4.Length == pEcDevice->m_pActiveEcFrame->cbLength )
							{
								PETYPE_EC_HEADER pActive = (PETYPE_EC_HEADER)ENDOF(pEcDevice->m_pActiveEcFrame);
								if ( pEcDevice->m_pActiveEcFrame->cbLength >= sizeof(ETYPE_EC_HEADER) )
								{
									memcpy(pActive, &p88A4->FirstEcHead, pEcDevice->m_pActiveEcFrame->cbLength);
									pActive->idx = pEcDevice->m_pActiveEcFrame->indexBak;
								}
								pEcDevice->m_pActiveEcFrame->result		= ECERR_NOERR;
							}
							else
								pEcDevice->m_pActiveEcFrame->result		= ECERR_DEVICE_INCOMPATIBLE;
							
							pEcDevice->m_pActiveEcFrame->received	= 1;
							return 1;
						}
					}
				}
			}		
		}
		break;

	

	default:
		break;
	}
	return 0;
}


long SendEcFrameEcDevice( PEcDevice pEcDevice, unsigned long invokeId, unsigned long cbLength, PETYPE_EC_HEADER pFrame)
{
	PEcUserFrameInfo pInfo = (PEcUserFrameInfo)new unsigned char[(sizeof(EcUserFrameInfo)+cbLength)];
	if ( pInfo )
	{
		unsigned char * pByte					= (unsigned char *)ENDOF(pInfo);		
		pInfo->invokeId				= invokeId;
		pInfo->received				= 0;		
		pInfo->result				= 0;
		pInfo->cbLength				= 0;
		pInfo->cmdCount				= 0;
		pInfo->timeout				= 50;	 
		memcpy(pByte, pFrame, cbLength);
		while ( cbLength-pInfo->cbLength > ETYPE_EC_HEADER_LEN )
		{
			PETYPE_EC_HEADER	pHead = (PETYPE_EC_HEADER)&pByte[pInfo->cbLength];
			if ( pInfo->cbLength == 0 )
			{	// first FLB cmd
				pInfo->indexBak	= pHead->idx;
				pHead->idx			= EC_HEAD_IDX_EXTERN_VALUE;
			}
			if ( ETYPE_EC_CMD_LEN(pHead) > cbLength-pInfo->cbLength )
				break;
			//if ( m_mapEcCmdInfo.Lookup(EcCmdHash(pHead), pCmdInfo) )
			//	break;

			if ( pInfo->cbLength == 0 )
				pInfo->ecCmdHash = EcCmdHash(pHead);
			pInfo->cbLength += ETYPE_EC_CMD_LEN(pHead);
			pInfo->cmdCount++;

			if ( cbLength-pInfo->cbLength > ETYPE_EC_HEADER_LEN )
				pHead->next = 1;
			else
				pHead->next = 0;												  
		}


		pEcDevice->m_pFifoEcFrame->Add(pInfo);
/*
		if ( cbLength = pInfo->cbLength ) // Modificato tullio 18 maggio 2010 era un solo =
		{
			pEcDevice->m_pFifoEcFrame->Add(pInfo);
		}
		else
		{
			delete pInfo;
			return ECERR_DEVICE_INVALIDDATA;
		}
*/

		return ECERR_NOERR;
	}
	else
	{
		return ECERR_DEVICE_NOMEMORY;
	}
}



//////////////////////////////////////////////////////////////////////
///\brief Queues an EtherCAT frames.
int QueueE88A4CmdEcDevice( PEcDevice pEcDevice, PETHERNET_88A4_FRAME pFrame)		
{		
	return pEcDevice->m_pFifoE88A4->Add(pFrame);		
}

//////////////////////////////////////////////////////////////////////////////////////////
///\brief Gets the state of the IO communication and checks if new frames have arrived. This method is called cyclically.
long GetIoStateEcDevice( PEcDevice pEcDevice )
{ 	
	// set device status info bits
	if ( pEcDevice->m_pDevState )
	{		
		SetResetMask(*pEcDevice->m_pDevState, ETHERNETRTMP_STATE_LINKERROR, pEcDevice->m_bLinkError);	
		SetResetMask(*pEcDevice->m_pDevState, ETHERNETRTMP_STATE_WATCHDOG, 0);
	}	

	long ret = IOSTATE_READY;

	if(
	    ( pEcDevice->m_pImage    ) )
	{			
		//check the incoming ethernet frames
		pEcDevice->m_pEcNpfDevice->CheckRecvFrames(); 
	}	
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
///\brief Starts the IO communications. This method is called cyclically.
long StartIoEcDevice( PEcDevice pEcDevice )
{  
	if ( pEcDevice->m_bLinkError ) 
		return IOSTATE_BUSY;

	long ret = IOSTATE_READY;
	
	if ( pEcDevice->m_pImage )
	{
		ret = IOSTATE_BUSY;

		//send cyclic input and output commands
		if( pEcDevice->m_pEcMaster )
			SendCycFramesEcMaster((PEcMaster)pEcDevice->m_pEcMaster);		
		
		// send queued E88A4 frames (from CEcMaster)
		SendQueuedE88A4CmdsEcDevice( pEcDevice);
		
		//Send queued user EtherCAT frames to network adpater
		SendQueuedEcCmdsEcDevice( pEcDevice);

		// reset watchdog
		pEcDevice->m_watchdogStartIo = 0; 		
	}

	return ret; 
}



//////////////////////////////////////////////////////////////////////
///\brief Update the link state of the network connection.	
void UpdateLinkStatusEcDevice( PEcDevice pEcDevice, unsigned long nTickNow)
{
	if ( (nTickNow % 50) == 0 )
	{	
		//get link status from network adapter
		unsigned long status = pEcDevice->m_pEcNpfDevice->GetMediaConnectStatus();
		if ( status == EcMediaStateConnected && pEcDevice->m_bLinkError )
		{
			pEcDevice->m_bLinkError = 0;

			if( pEcDevice->m_pDevState )
				SetResetMask(*pEcDevice->m_pDevState, ETHERNETRTMP_STATE_LINKERROR, pEcDevice->m_bLinkError);
		}		
		else if ( status == EcMediaStateDisconnected && !pEcDevice->m_bLinkError )
		{
			pEcDevice->m_bLinkError = 1;
			if ( pEcDevice->m_pDevState )
				SetResetMask(*pEcDevice->m_pDevState, ETHERNETRTMP_STATE_LINKERROR, pEcDevice->m_bLinkError);
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
///\brief Timer method called cyclically by the application.
int OnTimerEcDevice( PEcDevice pEcDevice )
{
	static int nTick=0;
	if( !pEcDevice->m_bOpened )
		return EC_E_INVALIDSTATE;
	

	UpdateLinkStatusEcDevice( pEcDevice, nTick);
	
	if ( pEcDevice->m_watchdogStartIo >= RTMP_WATCHDOG_CYCLES )
	{	
		if ( pEcDevice->m_watchdogStartIo == RTMP_WATCHDOG_CYCLES )
		{	
			SetResetMask(*pEcDevice->m_pDevState, ETHERNETRTMP_STATE_WATCHDOG, 1);
			pEcDevice->m_watchdogStartIo++;
		}
		pEcDevice->m_pEcNpfDevice->CheckRecvFrames( );			
		SendQueuedE88A4CmdsEcDevice( pEcDevice);
		SendQueuedEcCmdsEcDevice( pEcDevice);
	}
	else
	{
		pEcDevice->m_watchdogStartIo++;
	}		
	if( pEcDevice->m_pEcMaster )
		StateMachineEcMaster((PEcMaster) pEcDevice->m_pEcMaster ,1);

	if ( pEcDevice->m_pActiveEcFrame && pEcDevice->m_pActiveEcFrame->received == 1 )
	{	
		if ( pEcDevice->m_pCallback )
		{
			if( pEcDevice->m_pActiveEcFrame->result == ECERR_NOERR )
				OnReturningEcFrameEcSyncClient((PEcSyncClient) pEcDevice->m_pCallback, pEcDevice->m_pActiveEcFrame->result, pEcDevice->m_pActiveEcFrame->invokeId, pEcDevice->m_pActiveEcFrame->cbLength,(PETYPE_EC_HEADER)ENDOF(pEcDevice->m_pActiveEcFrame));
			else
				OnReturningEcFrameEcSyncClient((PEcSyncClient) pEcDevice->m_pCallback, pEcDevice->m_pActiveEcFrame->result, pEcDevice->m_pActiveEcFrame->invokeId, 0, NULL );
		}
					
		delete pEcDevice->m_pActiveEcFrame;
		pEcDevice->m_pActiveEcFrame = NULL;
	}

	if( pEcDevice->m_pEcMaster )
 		OnTimerEcMaster((PEcMaster)pEcDevice->m_pEcMaster,nTick++);	

	return 1;
}	


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


short Ricevi()
{
      unsigned short        RxBuffLen;     
      unsigned short	    RxPacketSize;
      unsigned char *       pRxPacketPointer;
      struct bpf_hdr	*   pHdr;              
    
      while  ((RxBuffLen = w5500.getRXReceivedSize(s)) != 0)                 // c'e' qualcosa nel buffer di ricezione?
      {                                                                      // si: stampa quanti byte ci sono
        
         w5500.recv_data_processing(s, (byte*)SocketBuffer, 2);                   // leggi i primi 2 byte
         w5500.execCmdSn(s, Sock_RECV);                                       // 
      
         RxPacketSize    = SocketBuffer[0];                                           // che indicano la lunghezza del pacchetto
         RxPacketSize  <<= 8;                                                    //
         RxPacketSize  &= 0xFF00;  
         RxPacketSize  |= (unsigned short)( (unsigned char)SocketBuffer[1] & 0x00FF);                                          //
         RxPacketSize  -= 2;                                                    // 
    

         w5500.recv_data_processing(s, (byte *)SocketBuffer, RxPacketSize);
         w5500.execCmdSn(s, Sock_RECV);
         
         pRxPacketPointer = SocketBuffer;

         iManda     = 1;
         iRinegozia = 1;
         iContaRinegozia = RINEGOZIA;
    
         if ( iTuttoEtherCAT == 70)
         {
        	if ( pEcDevice )
        	{ 
  			memset( cPacchettoRX[iContaRxPacchetti] , 0, sizeof(bpf_hdr) );
                        memcpy(&cPacchettoRX[iContaRxPacchetti][sizeof(bpf_hdr)], pRxPacketPointer, RxPacketSize);
  
  			pHdr = (struct bpf_hdr *)cPacchettoRX[iContaRxPacchetti];
  			pHdr->bh_caplen = RxPacketSize;
  			pHdr->bh_hdrlen = sizeof(bpf_hdr);
  									 										
  			pEcDevice->m_pEcNpfDevice->m_listPacket.Add((struct bpf_hdr *)&cPacchettoRX[iContaRxPacchetti]);	
  			iContaRxPacchetti++;
  			if ( iContaRxPacchetti >= MAX_PACCHETTI)
  			{
  			    iContaRxPacchetti = 0;
  			} 										
                 }
          }
      }
      return 1;
}

void MandaDoppioPacchetto( void * pData, unsigned int iLung )   // HHHH modificata tutta la funzione
{                                      
    w5500.send_data_processing(s, (byte *)pData, iLung);
    w5500.execCmdSn(s, Sock_SEND_MAC);    
}

