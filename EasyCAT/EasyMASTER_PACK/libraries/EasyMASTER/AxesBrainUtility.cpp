#include <SPI.h>         // needed for Ethernet library communication with the W5100 (Arduino ver>0018)

#include "AxesBrainEcInterface.h"
#include "Strutture.h"
#include "AxesBrainEcNpfDevice.h"
#include "AxesBrainEcDevice.h"

int                  nLast;
long                 nTicks;
extern int           iContaMilli;
extern PEcDevice     pEcDevice ;
extern PEcSyncClient pEcSyncClient;
short iChi;

void Start(void * pTime, unsigned int timeoutMsec);

void	CancellaEcMbSlave(PEcMbSlave pEcMbSlave); 

void	CancellaEcSlave(PEcSlave pEcSlave); 

extern int		RegisterMBoxPollingEcMaster( PEcMaster pEcMaster, EcMbSlave * pSlave, unsigned short slaveAddressMBoxState, int bUnregister);


unsigned long	EcCmdHash( PETYPE_EC_HEADER pHead)
{ 
	return pHead->cmdIdx + (pHead->len << 16);
}


void OsQueryTickCount( long * pTicks)
{
 	long  nDiff;
		
	nDiff = iContaMilli - nLast;
	if ( nDiff < 0 )
	{
		nDiff = 0 ;
	}
	nTicks += nDiff;
	if ( pTicks )
	{
 		*pTicks = nTicks;
 
	}
	nLast = iContaMilli;
}


///////////////////////////////////////////////////////////////////////////////
// TimeoutHelper

void Start(void * pTime, unsigned int timeoutMsec)
{	

	PTimeoutHelper pTimeoutHelper;
	long timeAdd;

	if ( !pTime  )
	{
		pTime = new TimeoutHelper; 	
	}
	pTimeoutHelper = ( PTimeoutHelper )pTime;

    OsQueryTickCount(&pTimeoutHelper->m_timeElapse);
	timeAdd	= timeoutMsec;
	pTimeoutHelper->m_timeElapse += timeAdd;
}



bool IsElapsed( void * pTime )
{
  	PTimeoutHelper pTimeoutHelper;

	if ( !pTime )
	{
       return 0;
	}

	pTimeoutHelper = ( PTimeoutHelper )pTime;

	long currentTime;

	OsQueryTickCount(&currentTime);
	if ( pTimeoutHelper->m_timeElapse < currentTime )
	{
          return true;
	}
    else    
   	{
          return false;
	}
}


unsigned char*	GetInputPtr( PIoImage pImage, unsigned long offs, unsigned long size )
{
	if ( pImage->m_pIn && (offs + size <= pImage->m_nIn) )
		return &pImage->m_pIn[offs];
	else
		return NULL;
}

unsigned char*	GetOutputPtr(  PIoImage pImage, unsigned long offs, unsigned long size )
{
	if ( pImage->m_pOut && (offs + size <= pImage->m_nOut) )
		return &pImage->m_pOut[offs];
	else
		return NULL;
}
unsigned char * ProcessDataPtr(PEcDevice  pEcDevice , int inOut, int offs, int size)
{
	if ( pEcDevice->m_pImage && inOut == VG_IN )
		return GetOutputPtr(pEcDevice->m_pImage, offs, size);
	else if ( pEcDevice->m_pImage && inOut == VG_OUT )
		return GetInputPtr( pEcDevice->m_pImage, offs, size);
	else
		return NULL;
}



PEcMailboxCmd MailboxAlloc(long size)
{ 
	PEcMailboxCmd pCmd = (PEcMailboxCmd) new unsigned char[size];
	return pCmd;
};
	
///\brief Frees a previously allocated mailbox command.
///
///\param pCmd Mailbox command to be freed.	
void MailboxFree(PEcMailboxCmd pCmd) 
{ 
	delete[] pCmd;
};


long SendMailboxCmd(PEcMailbox pEcMailbox , PEcMailboxCmd pCmd)
{
	if ( pCmd )
	{			
		if( pEcMailbox->m_pOwner )
			return OnMailboxReceiveEcMaster( (PEcMaster) pEcMailbox->m_pOwner , pCmd );
		else
		{			
			MailboxFree(pCmd);
			pCmd=NULL;
			return ECERR_DEVICE_UNKNOWNMAILBOXCMD;
		}
	}
	else
	{			
		return ECERR_DEVICE_UNKNOWNMAILBOXCMD;
	}
}


void CopiaSlave ( EcSlave*	pSlave,  unsigned char * pDati, unsigned long * pPointer  )
{

	unsigned long iPointer;
	iPointer = *pPointer;




	pSlave->m_rInitCmds				= 0;
	pSlave->m_cInitCmds				= INITCMD_INACTIVE;
	pSlave->m_nInitCmds				= 0;
	pSlave->m_pInitCmds				= NULL;
	pSlave->m_ppInitCmds			= NULL;

	for ( short ik=0; ik<EC_MAX_PORTS; ik ++)
	{
		pSlave->m_pEcSlaves[ik]=0;
	}

//<5555555>	

	memcpy( &pSlave->m_initcmdLen, &pDati[iPointer], sizeof(unsigned short));
	iPointer +=sizeof(unsigned short);

	memcpy( &pSlave->m_autoIncAddr, &pDati[iPointer], sizeof(unsigned short));
	iPointer +=sizeof(unsigned short);

	memcpy( &pSlave->m_physAddr, &pDati[iPointer], sizeof(unsigned short));
	iPointer +=sizeof(unsigned short);

	memcpy( &pSlave->m_nInitCmds, &pDati[iPointer], sizeof(unsigned short));
	iPointer +=sizeof(unsigned short);

	memcpy( &pSlave->m_szName, &pDati[iPointer],  ECAT_DEVICE_NAMESIZE+1);
	iPointer +=ECAT_DEVICE_NAMESIZE+1;

//<5------

	//simple slave without mailbox
	pSlave->m_slaveType		= simpleST;
	pSlave->m_currState		= DEVICE_STATE_INIT;
	pSlave->m_reqState		= pSlave->m_currState;	


	if ( pSlave->m_initcmdLen > 0 )
	{
		pSlave->m_pInitCmds		= (PEcInitCmdDesc)new unsigned char[pSlave->m_initcmdLen];
		pSlave->m_ppInitCmds	= new PEcInitCmdDesc[pSlave->m_nInitCmds];

		if ( pSlave->m_pInitCmds && pSlave->m_ppInitCmds )
		{

//------6>	
			memcpy( pSlave->m_pInitCmds, &pDati[iPointer], pSlave->m_initcmdLen);
			iPointer +=pSlave->m_initcmdLen;

			memcpy( &pSlave->m_prevPhysAddr, &pDati[iPointer],  sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);

			memcpy( &pSlave->m_prevPort, &pDati[iPointer],  sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);


			memcpy( &pSlave->m_bDcRefClock, &pDati[iPointer],  sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);
//<6------
			PEcInitCmdDesc	pInitCmds = pSlave->m_pInitCmds;

			for ( unsigned short i=0; i < pSlave->m_nInitCmds; i++ )
			{
				pSlave->m_ppInitCmds[i] = pInitCmds;
				pInitCmds = NEXT_EcInitCmdDesc(pInitCmds);
			}
		}
	}
	*pPointer=	iPointer;
}



void  DefinisciCopiaSlave( PEcDevice pEcDeviceFine, unsigned char * pDati, long i, unsigned long * pPointer ) 
{
	EcMaster*     pMaster;
	unsigned long iPointer;
	unsigned short iMerda;

	iPointer    =*pPointer;

	pMaster     = pEcDeviceFine->m_pEcMaster;


	EcSlave*		pSlave	= NULL;
	EcMbSlave*		pPort	= NULL;	
	short iT;

	pSlave = new EcSlave;
	memset( pSlave,0, sizeof(EcSlave));
	

//------5>

	memcpy( &pSlave->m_type, &pDati[iPointer], sizeof(unsigned short));
	iPointer +=sizeof(unsigned short);

//<555555>

	switch ( pSlave->m_type )
	{
	case ECAT_SLAVE_TYPE_SIMPLE:
		//Create slave object

		pSlave->m_pMaster    = 0;
		pSlave->m_physAddr   = 0 ;
		pSlave->m_autoIncAddr= 0;
		pSlave->m_currState	 = 0;
		pSlave->m_reqState	 = 0;
		pSlave->m_szName[0]	 = 0;		
		pSlave->m_tInitCmds	 = 0;
		pSlave->m_rInitCmds	 = 0;
		pSlave->m_cInitCmds	 = 0;
		pSlave->m_nInitCmds	 = 0;
		pSlave->m_pInitCmds	 = 0;
		pSlave->m_ppInitCmds = 0;
		for ( iT=0 ; iT<EC_MAX_PORTS; iT++)
		{
			pSlave->m_pEcSlaves[iT]=0;
		}
		pSlave->m_prevPhysAddr  = 0;
		pSlave->m_prevPort  = 0;
		pSlave->m_bDcRefClock  = 0;
		pSlave->m_regDlStatus			= 0;
		pSlave->m_nDcRefClockDelay928	= 0;
		pSlave->m_bDcTimingPending		= 0;
		memset(pSlave->m_nDcPortDelayT, 0, sizeof(pSlave->m_nDcPortDelayT));
		memset(pSlave->m_nDcPortDelayR, 0, sizeof(pSlave->m_nDcPortDelayR));
		pSlave->m_nDcExecDelay			= 30;
		memset(pSlave->m_nDcLineDelay, 0, sizeof(pSlave->m_nDcLineDelay));
		memset(pSlave->m_dcReceiveTime, 0, sizeof(pSlave->m_dcReceiveTime));

		pSlave->m_pMaster = pMaster;
 		CopiaSlave (pSlave, pDati, &iPointer);

	////////// TimeoutHelper /////////////////
		if ( pSlave->m_tInitCmds)
		{																
			delete pSlave->m_tInitCmds ;
			pSlave->m_tInitCmds = NULL;
		}	
		pSlave->m_tInitCmds = new TimeoutHelper;
		Start( pSlave->m_tInitCmds, 0 );

		pSlave->m_pEcMbSlave   = NULL;

		break;

	 case ECAT_SLAVE_TYPE_MAILBOX:
		//Create mailbox slave object
		pPort  = new EcMbSlave;
		memset(pPort,0 , sizeof(EcMbSlave));

//		memset(&pSlave->m_slaveType,0 , sizeof(0));
		pSlave->m_pMaster    = 0;
		pSlave->m_physAddr   = 0 ;
		pSlave->m_autoIncAddr= 0;
		pSlave->m_currState	 = 0;
		pSlave->m_reqState	 = 0;
		pSlave->m_szName[0]	 = 0;		
		pSlave->m_tInitCmds	 = 0;
		pSlave->m_rInitCmds	 = 0;
		pSlave->m_cInitCmds	 = 0;
		pSlave->m_nInitCmds	 = 0;
		pSlave->m_pInitCmds	 = 0;
		pSlave->m_ppInitCmds = 0;
		for ( iT=0 ; iT<EC_MAX_PORTS; iT++)
		{
			pSlave->m_pEcSlaves[iT]=0;
		}
		pSlave->m_prevPhysAddr  = 0;
		pSlave->m_prevPort  = 0;
		pSlave->m_bDcRefClock  = 0;
		pSlave->m_regDlStatus			= 0;
		pSlave->m_nDcRefClockDelay928	= 0;
		pSlave->m_bDcTimingPending		= 0;
		memset(pSlave->m_nDcPortDelayT, 0, sizeof(pSlave->m_nDcPortDelayT));
		memset(pSlave->m_nDcPortDelayR, 0, sizeof(pSlave->m_nDcPortDelayR));
		pSlave->m_nDcExecDelay			= 30;
		memset(pSlave->m_nDcLineDelay, 0, sizeof(pSlave->m_nDcLineDelay));
		memset(pSlave->m_dcReceiveTime, 0, sizeof(pSlave->m_dcReceiveTime));
		pSlave->m_pMaster	= pMaster;
		CopiaSlave (pSlave, pDati, &iPointer);

		pPort->m_actTransition = 0;
		pPort->m_pEcSlave = 0;
		pPort->m_pCoE     = 0;
		pPort->m_pFoE     = 0;
		pPort->m_pSoE     = 0;

//------7>	
		memcpy( &pPort->m_pCoE , &pDati[iPointer], sizeof(unsigned long));
		iPointer +=sizeof(unsigned long);
		memcpy( &pPort->m_pFoE , &pDati[iPointer], sizeof(unsigned long));
		iPointer +=sizeof(unsigned long);
		memcpy( &pPort->m_pSoE , &pDati[iPointer], sizeof(unsigned long));
		iPointer +=sizeof(unsigned long);
//<77777>
		
		{
			if ( pPort->m_pCoE )
			{//EtherCAT slaves supports the CANopen over EtherCAT protocol
				pPort->m_pCoE = new ECMBSLAVE_COE_SUPPORT;
				if ( pPort->m_pCoE )
				{
					memset(pPort->m_pCoE, 0, sizeof(ECMBSLAVE_COE_SUPPORT));
					pPort->m_pCoE->pPend	= new CFiFoListDyn<PEcMailboxCmd>(10);			
					if ( pPort->m_pCoE->pPend == NULL )
					{
						safe_delete(pPort->m_pCoE->pPend);				
						safe_delete(pPort->m_pCoE);
					}
				}
			}
			if ( pPort->m_pFoE  )
			{
				pPort->m_pFoE = new ECMBSLAVE_FOE_SUPPORT;
				if ( pPort->m_pFoE )
				{
					memset(pPort->m_pFoE, 0, sizeof(ECMBSLAVE_FOE_SUPPORT));
					pPort->m_pFoE->pPend	= new CFiFoListDyn<PEcMailboxCmd>(1);			
					if ( pPort->m_pFoE->pPend == NULL )
					{
						safe_delete(pPort->m_pFoE->pPend);				
						safe_delete(pPort->m_pFoE);
					}
				}
			}
			if (pPort->m_pSoE  )
			{
				pPort->m_pSoE = new ECMBSLAVE_SOE_SUPPORT;
				if ( pPort->m_pSoE )
				{
					memset(pPort->m_pSoE, 0, sizeof(ECMBSLAVE_SOE_SUPPORT));
					pPort->m_pSoE->pPend = new CFiFoListDyn<PEcMailboxCmd>(20);
					pPort->m_pSoE->pCmds	= new CHashTableDyn<unsigned short, ECMBSLAVE_SOE_CMD_INFO>(10);
					if ( pPort->m_pSoE->pPend == NULL  || pPort->m_pSoE->pCmds == NULL )
					{
						safe_delete(pPort->m_pSoE->pPend);
						safe_delete(pPort->m_pSoE->pCmds);
						safe_delete(pPort->m_pSoE);
					}
				}
			}
		}
		
		pPort->m_mbxOStart[0] = 0;
		pPort->m_mbxOStart[1] = 0;
		pPort->m_mbxOLen[0]   = 0;
		pPort->m_mbxOLen[1]   = 0;
		pPort->m_mbxIStart[0] = 0;
		pPort->m_mbxIStart[1] = 0;
		pPort->m_mbxILen[0]   = 0;	
		pPort->m_mbxILen[1]   = 0;
		
		pPort->m_mbxOutShortSend = 0;
		pPort->m_mbxIdx          = 0;
		pPort->m_cycleMbxPolling = 0;
		pPort->m_stateMbxPolling = 0;

		pPort->m_tMbxCmds  = 0;
		pPort->m_rMbxCmds  = 0;
		pPort->m_cMbxCmds  = INITCMD_INACTIVE;
		pPort->m_nMbxCmds  = 0;
		pPort->m_oMbxCmds  = 0;
		pPort->m_pMbxCmds  = 0;
		pPort->m_ppMbxCmds = 0;
		pPort->m_slaveAddressMBoxState = 0;		// bit offset in logical area
		pPort->m_cycleMbxPollingTime   = 0;		// cycle time for MBox polling


		{


//77777777>	
			memcpy( &pPort->m_mbxOStart[0] , &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);
			memcpy( &pPort->m_mbxOLen[0] , &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);
			memcpy( &pPort->m_mbxIStart[0] , &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);
			memcpy( &pPort->m_mbxILen[0] , &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);
			memcpy( &pPort->m_mbxOStart[1] , &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);
			memcpy( &pPort->m_mbxOLen[1] , &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);
			memcpy( &pPort->m_mbxIStart[1] , &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);
			memcpy( &pPort->m_mbxILen[1] , &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);
			memcpy( &pPort->m_cycleMbxPollingTime , &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);


			memcpy( &iMerda , &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);
			if ( iMerda )
			{
				pPort->m_mbxOutShortSend = 1;
			}
			else
			{
				pPort->m_mbxOutShortSend = 0;

			}

			memcpy( &iMerda ,  &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);
			if ( iMerda )
			{
				pPort->m_cycleMbxPolling = 1;
			}
			else
			{
				pPort->m_cycleMbxPolling = 0;

			}
			memcpy( &iMerda,  &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);
			if ( iMerda )
			{
				pPort->m_stateMbxPolling = 1;
			}
			else
			{
				pPort->m_stateMbxPolling = 0;

			}
			
			memcpy( &pPort->m_slaveAddressMBoxState , &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);
			memcpy( &pPort->m_mboxCmdLen	 , &pDati[iPointer], sizeof(unsigned int));
			iPointer +=sizeof(unsigned int);
			memcpy( &pPort->m_nMbxCmds	 , &pDati[iPointer], sizeof(unsigned short));
			iPointer +=sizeof(unsigned short);


//<7------


			if ( pPort->m_nMbxCmds > 0 )
			{
				pPort->m_pMbxCmds	= (PEcMailboxCmdDesc)new unsigned char[pPort->m_mboxCmdLen];
				memset(pPort->m_pMbxCmds, 0, pPort->m_mboxCmdLen );
				pPort->m_ppMbxCmds	= new PEcMailboxCmdDesc[pPort->m_nMbxCmds];
//				memset(pPort->m_ppMbxCmds, 0, sizeof(PEcMailboxCmdDesc)*pPort->m_nMbxCmds );
				if ( pPort->m_pMbxCmds && pPort->m_ppMbxCmds )
				{
//------8>	
					memcpy( pPort->m_pMbxCmds, &pDati[iPointer], pPort->m_mboxCmdLen);
					iPointer +=pPort->m_mboxCmdLen;
//<8------
					PEcMailboxCmdDesc	pMbxCmds = pPort->m_pMbxCmds;
					for ( unsigned short i=0; i < pPort->m_nMbxCmds; i++ )
					{
						pPort->m_ppMbxCmds[i] = pMbxCmds;

					 	pMbxCmds = NEXT_EcMailboxCmdDesc(pMbxCmds);
					}
				}
				else
					pPort->m_nMbxCmds		= 0;
			}
		}	

		if ( pPort->m_stateMbxPolling )
			//inform master that the mailbox state should be checked cyclically
			RegisterMBoxPollingEcMaster( pMaster , pPort , pPort->m_slaveAddressMBoxState, 0);

	////////// TimeoutHelper /////////////////
		if ( pSlave->m_tInitCmds)
		{																
			delete pSlave->m_tInitCmds ;
			pSlave->m_tInitCmds = NULL;
		}	
		pSlave->m_tInitCmds = new TimeoutHelper;
		Start( pSlave->m_tInitCmds, 0);

	////////// TimeoutHelper /////////////////
		if ( pPort->m_tMbxCmds)
		{																
			delete pPort->m_tMbxCmds ;
			pPort->m_tMbxCmds = NULL;
		}	
		pPort->m_tMbxCmds = new TimeoutHelper;
		Start( pPort->m_tMbxCmds, 0);




	////////// TimeoutHelper /////////////////
		if ( pPort->m_pCoE )
		{																
			pPort->m_pCoE->safe.timeout = new TimeoutHelper;
			Start( pPort->m_pCoE->safe.timeout, 0);
		}	

	////////// TimeoutHelper /////////////////
		if ( pPort->m_pFoE )
		{																
			pPort->m_pFoE->safe.timeout = new TimeoutHelper;
			Start( pPort->m_pFoE->safe.timeout, 0);
			pPort->m_pFoE->safe.busy = new TimeoutHelper;
			Start( pPort->m_pFoE->safe.busy, 0);
		}	

	////////// TimeoutHelper /////////////////
		if ( pPort->m_pSoE )
		{																
			pPort->m_pSoE->safe.timeout = new TimeoutHelper;
			Start( pPort->m_pSoE->safe.timeout, 0);
		}	


		pSlave->m_pEcMbSlave  = pPort;
		pPort->m_pEcSlave     = pSlave;

		break;
	default:		
		*pPointer=	iPointer;
		return ;
	}
	if ( pSlave )
	{
		//Try to add slave to hash table
		if ( pMaster->m_pEcSlave->Add(pSlave->m_physAddr, pSlave) )
		{  
			if (pSlave->m_bDcRefClock)
				pMaster->m_pRefClock = pSlave;

			pMaster->m_ppEcSlave[i] = pSlave;
			if ( pPort )
			{  
				pMaster->m_ppEcMbSlave[pMaster->m_nEcMbSlave++] = pPort;
			}
		}
		else
		{	
			delete pSlave;
			pSlave = NULL;
		}
	}
	*pPointer=	iPointer;
};



void CancellaEcSlave(PEcSlave pEcSlave) 
{

	safe_delete_a(pEcSlave->m_ppInitCmds);
	safe_delete_a(pEcSlave->m_pInitCmds);

}



unsigned int CancellaEcDevice(  )
{
	unsigned int  iT;

	if ( pEcDevice->m_pEcNpfDevice )
	{
		void * pData;
		while( pEcDevice->m_pEcNpfDevice->m_listPacket.Remove(pData) )
 		{
		}
 
		delete pEcDevice->m_pEcNpfDevice;
		pEcDevice->m_pEcNpfDevice = NULL;

	}
	
 //// Delete puntatori  //////
	
	PETHERNET_88A4_FRAME p88A4;
	if ( pEcDevice->m_pFifoE88A4 )
	{
		while (pEcDevice->m_pFifoE88A4->Remove(p88A4))
		{
		}
		safe_delete(pEcDevice->m_pFifoE88A4);		
	}
	
	PEcUserFrameInfo pInfo;
	if ( pEcDevice->m_pFifoEcFrame )
	{
		while (pEcDevice->m_pFifoEcFrame->Remove(pInfo))
			delete pInfo;
		safe_delete(pEcDevice->m_pFifoEcFrame);
	}

	if( pEcDevice->m_pActiveEcFrame )
		delete pEcDevice->m_pActiveEcFrame;

	if ( pEcDevice->m_pEcMaster )
	{
		if ( pEcDevice->m_pEcMaster->m_tCyclicSend )
		{
			delete pEcDevice->m_pEcMaster->m_tCyclicSend;
			pEcDevice->m_pEcMaster->m_tCyclicSend = NULL;
		}
		if ( pEcDevice->m_pEcMaster->m_tInitCmds )
		{
			delete pEcDevice->m_pEcMaster->m_tInitCmds;
			pEcDevice->m_pEcMaster->m_tInitCmds = NULL;
		}


		////////// TimeoutHelper /////////////////
		if ( pEcDevice->m_pEcMaster->m_pInfoBuffer->timeout)
		{																
			delete pEcDevice->m_pEcMaster->m_pInfoBuffer->timeout ;
			pEcDevice->m_pEcMaster->m_pInfoBuffer->timeout = NULL;
		}	

		////////// TimeoutHelper /////////////////
		if (  pEcDevice->m_pEcMaster->m_pPend )
		{																
			if ( pEcDevice->m_pEcMaster->m_pPend->timeout)
			{																
				delete pEcDevice->m_pEcMaster->m_pPend->timeout ;
				pEcDevice->m_pEcMaster->m_pPend->timeout = NULL;
			}	
		}	


	// Delete master //
		{

	////delete slave  //////		
			{
				EcSlave*		pSlave	= NULL;
				void *          p;

				p = NULL;
				while ( pEcDevice->m_pEcMaster->m_pEcSlave->GetNextEntry( pSlave , p )  )
				{
					// delete Slave
					safe_delete_a(pSlave->m_ppInitCmds);
					safe_delete_a(pSlave->m_pInitCmds);

				}
			}
			pEcDevice->m_pEcMaster->m_pEcSlave->RemoveAll();
			safe_delete(pEcDevice->m_pEcMaster->m_pEcSlave);




			
			for ( iT=0; iT<pEcDevice->m_pEcMaster->m_nEcMbSlave; iT++)
			{
				////////// TimeoutHelper /////////////////
				if ( pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_tMbxCmds )
				{
					delete pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_tMbxCmds;
					pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_tMbxCmds = NULL;
				}
				////////// TimeoutHelper /////////////////
				if ( pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pCoE )
				{
					if ( pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pCoE->safe.timeout )
					{
						delete pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pCoE->safe.timeout;
						pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pCoE->safe.timeout = NULL;
					}
				}
				////////// TimeoutHelper /////////////////
				if ( pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pFoE )
				{
					if ( pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pFoE->safe.timeout  )
					{
						delete pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pFoE->safe.timeout ;
						pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pFoE->safe.timeout  = NULL;
					}
				}
				////////// TimeoutHelper /////////////////
				if ( pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pFoE  )
				{
					if ( pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pFoE->safe.busy  )
					{
						delete pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pFoE->safe.busy ;
						pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pFoE->safe.busy  = NULL;
					}
				}
				////////// TimeoutHelper /////////////////
				if ( pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pSoE  )
				{
					if ( pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pSoE->safe.timeout  )
					{
						delete pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pSoE->safe.timeout ;
						pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]->m_pSoE->safe.timeout  = NULL;
					}
				}

				CancellaEcMbSlave(pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]);
				safe_delete(pEcDevice->m_pEcMaster->m_ppEcMbSlave[iT]);
			}
			safe_delete(pEcDevice->m_pEcMaster->m_ppEcMbSlave);	

			for ( iT=0; iT< pEcDevice->m_pEcMaster->m_nEcSlave; iT++)
			{
				////////// TimeoutHelper /////////////////
				if ( pEcDevice->m_pEcMaster->m_ppEcSlave[iT]->m_tInitCmds )
				{
					delete pEcDevice->m_pEcMaster->m_ppEcSlave[iT]->m_tInitCmds;
					pEcDevice->m_pEcMaster->m_ppEcSlave[iT]->m_tInitCmds = NULL;
				}

				CancellaEcSlave(pEcDevice->m_pEcMaster->m_ppEcSlave[iT]);
				safe_delete(pEcDevice->m_pEcMaster->m_ppEcSlave[iT]);
			}
			safe_delete(pEcDevice->m_pEcMaster->m_ppEcSlave);	
			
			safe_delete(pEcDevice->m_pEcMaster->m_ppEcPortMBoxState);

	////delete frame  //////		
			EcAcycFrameInfo info;
			while ( pEcDevice->m_pEcMaster->m_listAcycFrame.Remove(info) )
			{
				if( info.pFrame )
				{
					struct bpf_hdr* pHdr = (struct bpf_hdr*)((unsigned char*)info.pFrame - sizeof(bpf_hdr));
					delete pHdr;
				}
			}

	////delete informazioni cicliche  //////		

			for (unsigned int i=0; i < pEcDevice->m_pEcMaster->m_nEcCycInfo; i++ )
			{
				safe_delete(pEcDevice->m_pEcMaster->m_ppEcCycInfo[i]);
			}
			safe_delete(pEcDevice->m_pEcMaster->m_ppEcCycInfo);

		}

	////delete master  //////		
		safe_delete_a(pEcDevice->m_pEcMaster->m_ppInitCmds);
		safe_delete_a(pEcDevice->m_pEcMaster->m_pInitCmds);

		safe_delete(pEcDevice->m_pEcMaster->m_pInfoBuffer);
		safe_delete(pEcDevice->m_pEcMaster);
	}

// Cancella io imagemaster //
	if ( pEcDevice->m_pImage )
	{
	 	safe_delete(pEcDevice->m_pImage->m_pOut);
	 	safe_delete(pEcDevice->m_pImage->m_pIn);
		safe_delete(pEcDevice->m_pImage);
	}
	return 1;
}


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////


