#include <SPI.h>         // needed for Ethernet library communication with the W5100 (Arduino ver>0018)

#include "EthercatInterface.h"
#include "AxesBrainEcInterface.h"
#include "Strutture.h"
#include "AxesBrainEcNpfDevice.h"
#include "AxesBrainEcDevice.h"

int StartIoEcDeviceExecute( void *  pEcDeviceExecute );
unsigned int CancellaEcDevice(  );
unsigned int OpenEcDevice(PEcDevice pEcDevice);	
unsigned int CloseEcDevice(PEcDevice pEcDevice);

extern unsigned char  cMacAddress[6];
extern int			  nLast;
//extern LARGE_INTEGER  nTicks;
extern long  nTicks;
extern unsigned char  iCampionaturaDaEthernet;
extern unsigned char  iSiScambio;


bool		SysTimeRead = 0;
int		LenEcatSysTime;	
unsigned int	iPrecedente;	 
short    	iRinegozia ;
short		iAggancia;
unsigned char	iCampionaturaDaEthernet;
unsigned char	iSiScambio;
short           iManda;
unsigned char   cMacAddress[6];
int		m_nAdapter;	
int		Ttune = 0;
int             iContaCount=0;
int		iContaMilli=0;
int				iTuttoEtherCAT;
int				iEtherCATOperationale;;
int				iEtherCATConnesso;
int				iEtherCATErrore;
int				iCollegatoEtherCAT;
int				iFaseEtherCAT;
int				iCuntaFaseEtherCAT;
int				iMinchiaEtherCAT;
int				iCuntaScansioneEtherCAT;
int				iContaLoopEtherCAT;

	
PEcDevice	pEcDevice     ;
PEcSyncClient	pEcSyncClient ;


short  Ricevi();

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////
////  Creazione e cancellazione  EcDevice e EcSync ///////////////         
//////////////////////////////////////////////////////////////////



int DeleteEcDeviceExecute( )
{
 
	if ( pEcDevice )
	{
		CancellaEcDevice(  );
		if ( pEcDevice )
		{
			delete(pEcDevice);
			pEcDevice = NULL;
		}
	}
 	
	return 1;
}

int DeleteEcSyncExecute(  void  )
{
 
	if ( pEcSyncClient )
	{
		delete pEcSyncClient;
		pEcSyncClient = NULL;
	}
 	
	return 1;
}

//////////////////////////////////////////////////////////////////
/////      Copiatura   di   EcDevice e EcSync      ///////////////         
//////////////////////////////////////////////////////////////////

int CaricaEcSyncExecute(  
						 void *  pMasterExecute
					   )
{
 
	PEcMailbox	pMailbox;	
	PEcMaster	pMaster;
	
	pEcSyncClient = new EcSyncClient;



	pEcSyncClient->m_pMailbox = 0;
	pEcSyncClient->m_pDevice = 0;
	pEcSyncClient->m_nTimeout= 0;
	memset( &pEcSyncClient->m_syncDevReq, 0, sizeof(EcSyncDevReq));
	memset( &pEcSyncClient->m_syncMbReq, 0, sizeof(EcSyncMbReq));
	pEcSyncClient->m_bWindowsTimer = 0;
	pEcSyncClient->m_nInvokeId = 0;



	pEcSyncClient->m_bWindowsTimer = 1;
	pEcSyncClient->m_nInvokeId = 0;

	pEcSyncClient->m_nTimeout = DEFAULT_SYNCTIMEOUT;
 	if( pEcDevice )
	{
		pEcSyncClient->m_pDevice = pEcDevice;
//////////!!!!!!!!!!!!!!!///////// 		pEcDevice->m_pCallback   = &EcDeviceCallback;		
	}
	pMaster  = (PEcMaster)pMasterExecute;

	pMailbox = &pMaster->m_mailbox;
	if( pMailbox )
	{
		pEcSyncClient->m_pMailbox = pMailbox;	
 		pMailbox->m_pClient       = pEcSyncClient;		
	}

 
	return 1;
}



/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

int GetMacAddressEcDeviceExecute(  void *          pEcDeviceExecute,
									unsigned char * caddr
								)
{
	memcpy( caddr, &((PEcDevice)pEcDeviceExecute)->m_macAdapter , sizeof(ETHERNET_ADDRESS));
	return 1;
}

unsigned long  ProcessDataSizeEcDeviceExecute(  void *  pEcDeviceExecute ,
												short   iTipo0In1Out
												)
{
	if ( ((PEcDevice)pEcDeviceExecute)->m_pImage  && iTipo0In1Out == 0 )
		return ((PEcDevice)pEcDeviceExecute)->m_pImage->m_nIn;
	else if ( ((PEcDevice)pEcDeviceExecute)->m_pImage && iTipo0In1Out == 1 )
		return ((PEcDevice)pEcDeviceExecute)->m_pImage->m_nOut;
	else
		return -1;
}

unsigned char *  ProcessDataPtrEcDeviceExecute(   void *         pEcDeviceExecute ,
												   short          iTipo0In1Out,
												   unsigned long  iOffset,
												   unsigned long  iNumeroByte
											   )
{
	if ( ((PEcDevice)pEcDeviceExecute)->m_pImage  && iTipo0In1Out == 0 )
		return ((PEcDevice)pEcDeviceExecute)->m_pImage->m_pOut+iOffset;
	else if ( ((PEcDevice)pEcDeviceExecute)->m_pImage && iTipo0In1Out == 1 )
		return ((PEcDevice)pEcDeviceExecute)->m_pImage->m_pIn+iOffset;
	else
		return NULL;
}



void *  GetMasterPointerExecute( void *  pEcDeviceExecute )
{
	return ((PEcDevice)pEcDeviceExecute)->m_pEcMaster;
}






//////////////////////////////////////////////////////////////////
/////////////         Operazioni  su Master          ////////////
//////////////////////////////////////////////////////////////////
int GetStateMachineMasterExecute( void *  pMasterPointerExecute  )
{
	switch (((PEcMaster)pMasterPointerExecute)->m_currState)
	{
		case EC_MASTER_STATE_INIT:
		case EC_MASTER_STATE_WAIT_SLAVE_I_P:
		case EC_MASTER_STATE_WAIT_SLAVE_P_I:
		case EC_MASTER_STATE_WAIT_SLAVE_S_I:
		case EC_MASTER_STATE_WAIT_SLAVE_O_I:
			return DEVICE_STATE_INIT;

		case EC_MASTER_STATE_PREOP:
		case EC_MASTER_STATE_WAIT_SLAVE_P_S:
		case EC_MASTER_STATE_WAIT_SLAVE_S_P:
		case EC_MASTER_STATE_WAIT_SLAVE_O_P:
			return DEVICE_STATE_PREOP;

		case EC_MASTER_STATE_SAFEOP:
		case EC_MASTER_STATE_WAIT_SLAVE_S_O:
		case EC_MASTER_STATE_WAIT_SLAVE_O_S:
			return DEVICE_STATE_SAFEOP;

		case EC_MASTER_STATE_OP:
			return DEVICE_STATE_OP;
	}
	return 0;
}


int RequestStateMasterExecute( void *  pMasterPointerExecute,
								unsigned short state 
													      )
{
	switch ( state )
	{
		case DEVICE_STATE_INIT:
		case DEVICE_STATE_PREOP:
		case DEVICE_STATE_SAFEOP:
		case DEVICE_STATE_OP:
		((PEcMaster)pMasterPointerExecute)->m_reqState = state;
		break;
	}
	return 1;
}





int GetIoStateEcDeviceExecute(  void *  pEcDeviceExecute )
{

	if ( pEcDevice )
	{
		return GetIoStateEcDevice( pEcDevice );

	}
	else
	{
		return 0;
	}
}

int StartIoEcDeviceExecute( void *  pEcDeviceExecute )
{
	if ( pEcDevice )
	{
		return StartIoEcDevice( pEcDevice );
	}
	else
	{
		return 0;
	}
}

int OnTimerEcDeviceExecute(  void *  pEcDeviceExecute )
{
	if ( pEcDevice  )
	{
		return OnTimerEcDevice( pEcDevice );
	}
	else
	{
		return 0;
	}
}






/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void ApriEtherCAT( void )
{

	short i;

	pEcDevice		= NULL;
	pEcSyncClient		= NULL;


	iTuttoEtherCAT		= 0;
	iFaseEtherCAT		= 1;
	iEtherCATOperationale	= 0;
	iEtherCATConnesso	= 0;
	iEtherCATErrore		= 0;
	iCuntaFaseEtherCAT	= 0;
	iCuntaScansioneEtherCAT = 0;
	iContaLoopEtherCAT      = 0;
}



void ChiudiEtherCAT( void )
{	

	short i;

 	if( pEcSyncClient )
	{
		DeleteEcSyncExecute( ) ;	
		pEcSyncClient = NULL;
	}
	
	if( pEcDevice )
	{
		DeleteEcDeviceExecute(  );		
		pEcDevice = NULL;
	}

	pEcDevice			= NULL;
	pEcSyncClient		= NULL;



	iFaseEtherCAT		= -1;
	iCuntaFaseEtherCAT	= 0;
	iCuntaScansioneEtherCAT = 0;
	iContaLoopEtherCAT      = 0;
}


unsigned char ComandiEtherCAT(  unsigned char * pComando , unsigned char * pDati)
{
	return 1;
}

void  DefinisciCopiaSlave( PEcDevice pEcDeviceFine, unsigned char * pDati, long i, unsigned long * pPointer ); 

unsigned char TrasferisciConfigurazioneEtherCAT( unsigned char * pDati, unsigned long dwInputBufferSize )
{

	PEcMaster     pMaster;

	unsigned long iPointer ;
	unsigned long iVal ;

	pEcDevice = new EcDevice;
	
//-------1>	
 	iVal = sizeof(EcDevice);
	memcpy( &pEcDevice->m_macAdapter, pDati, sizeof(ETHERNET_ADDRESS) );

	memcpy( &pEcDevice->m_macAdapter, &cMacAddress, sizeof(ETHERNET_ADDRESS) );
		
	iPointer = sizeof(ETHERNET_ADDRESS);
//<1------




	pEcDevice ->m_pImage = 0;
	pEcDevice ->m_pEcMaster = 0;	
	pEcDevice ->m_pCallback = 0;
	pEcDevice ->m_bOpened = 0;
	pEcDevice ->m_watchdogStartIo = 0;
	pEcDevice ->m_bLinkError = 0;
	pEcDevice ->m_pDevState = 0;
	pEcDevice ->m_pDevCtrl = 0;		
	pEcDevice ->m_pActiveEcFrame = 0;

	pEcDevice ->m_hReceiverThread = 0;
	pEcDevice ->m_dwThreadId = 0;
	pEcDevice ->m_bStopReceiver = 0;
	pEcDevice ->m_lRef = 0;
	pEcDevice ->m_pszAdapter = 0;
	pEcDevice ->m_pAdapter = 0;			

//    pEcDevice->m_pFifoE88A4 =0 ;
//    pEcDevice->m_pFifoEcFrame =0 ;

/////////////////////////////////////////////////////////////
////////   Caricamento         PIoImage             /////////
/////////////////////////////////////////////////////////////
	pEcDevice->m_pImage = new IoImage;

//////////////////////////////////////////////////////
// Azzeramento dela struttura

	pEcDevice->m_pImage->m_nIn  = 0;
	pEcDevice->m_pImage->m_nOut = 0;
	pEcDevice->m_pImage->m_pIn	= NULL;
	pEcDevice->m_pImage->m_pOut	= NULL;

	iFaseEtherCAT = 0;
 	pEcDevice->m_pEcNpfDevice =  new CEcNpfDevice();


	if ( dwInputBufferSize > sizeof(ETHERNET_ADDRESS) ) 
	{

		iFaseEtherCAT = 1;
	//------2>	
 		iVal  = sizeof(IoImage);
		memcpy( pEcDevice->m_pImage , &pDati[iPointer], sizeof(IoImage));
 		iPointer += sizeof(IoImage);
	//<2------

		pEcDevice->m_pImage->m_pIn	= NULL;
		pEcDevice->m_pImage->m_pOut	= NULL;


		if ( pEcDevice->m_pImage->m_nIn > 0 )
		{
			pEcDevice->m_pImage->m_pIn	= new unsigned char[pEcDevice->m_pImage->m_nIn];
			memset( pEcDevice->m_pImage->m_pIn, 0, pEcDevice->m_pImage->m_nIn );
		}
		if ( pEcDevice->m_pImage->m_nOut > 0 )
		{
			pEcDevice->m_pImage->m_pOut = new unsigned char[pEcDevice->m_pImage->m_nOut];
			memset( pEcDevice->m_pImage->m_pOut, 0, pEcDevice->m_pImage->m_nOut );
		}

		pEcDevice->m_pFifoE88A4		= new CFiFoList<PETHERNET_88A4_FRAME, ETHERNETRTMP_MAXE88A4FIFO>();	
		pEcDevice->m_pFifoEcFrame	= new CFiFoList<PEcUserFrameInfo, ETHERNETRTMP_MAXECUSERFIFO>();

		pMaster   = new EcMaster;

	//////////////////////////////////////////////////////
	// Azzeramento dela struttura
		
		pMaster->m_ipDev   = 0;	
		memset( &pMaster->m_macDest ,0 ,sizeof(ETHERNET_ADDRESS));
		memset( &pMaster->m_macSrc ,0 ,sizeof(ETHERNET_ADDRESS));
		pMaster->m_szName[0] = '\0';	
		pMaster->m_pEcSlave = 0;
		pMaster->m_nEcSlave = 0;
		pMaster->m_nEcMbSlave =0 ;
		pMaster->m_ppEcSlave = 0;
		pMaster->m_ppEcMbSlave = 0;
		pMaster->m_tCyclicSend =0;
		memset( &pMaster->m_currState ,0 ,sizeof(EC_MASTER_STATE));
		pMaster->m_reqState = 0;
		pMaster->m_ioOpenStateInit = 0; 
		pMaster->m_tInitCmds = 0;
		pMaster->m_cInitCmds = 0;
		pMaster->m_nInitCmds = 0;
		pMaster->m_pInitCmds = 0;
		pMaster->m_ppInitCmds = 0;
		pMaster->m_maxAsyncFrameSize= 0;
		pMaster->m_logAddressMBoxStates = 0;		// start address of log. area of sync man states
		pMaster->m_sizeAddressMBoxStates =0 ;	// size of log. area of sync man states
		pMaster->m_ppEcPortMBoxState = 0;
		memset( &pMaster->m_listFree ,0 ,sizeof(LIST_ENTRY_AB));
		memset( &pMaster->m_listPend ,0 ,sizeof(LIST_ENTRY_AB));
		pMaster->m_pInfoBuffer = 0;
		pMaster->m_pPend = 0;
		pMaster->m_ppEcCycInfo = 0;
		pMaster->m_maxEcCycInfo = 0;
		pMaster->m_nEcCycInfo = 0;
		memset( &pMaster->m_mailbox ,0 ,sizeof(EcMailbox));
		pMaster->m_pRefClock = NULL;
		pMaster->m_dcSyncMode = 0;
		pMaster->m_nDcTimingCounter = 0;
		pMaster->m_nDcTimingSendArmw = 0;
		pMaster->m_eDcState = 0;
		pMaster->m_enableContinuousMeasuring = 1;        //Fa il ricalcolo DCDCDCDC anche durante  lo stato di operazionale
	// 	pMaster->m_enableContinuousMeasuring = 0;

	// Caricamento Master

	//------3>	

		memcpy(&pMaster->m_mailbox.m_addr.id , &pDati[iPointer], sizeof( unsigned short ));
 		iPointer += sizeof( unsigned short );
		memcpy(&pMaster->m_mailbox.m_addr.port , &pDati[iPointer], sizeof( unsigned short ));
 		iPointer += sizeof( unsigned short );
		memcpy(&pMaster->m_mailbox.m_pClient , &pDati[iPointer], sizeof( void * ));
 		iPointer += sizeof( void *  );

		pMaster->m_mailbox.m_pOwner     = pMaster;

		memcpy(&pMaster->m_maxSlaves , &pDati[iPointer], sizeof( unsigned long ));
 		iPointer += sizeof( unsigned long  );

		memcpy(&pMaster->m_initcmdLen , &pDati[iPointer], sizeof( unsigned long ));
 		iPointer += sizeof( unsigned long  );
		memcpy(&pMaster->m_nEcSlave , &pDati[iPointer], sizeof( unsigned long ));
 		iPointer += sizeof( unsigned long  );
		memcpy(&pMaster->m_nEcMbSlave , &pDati[iPointer], sizeof( unsigned long ));
 		iPointer += sizeof( unsigned long  );
		

		pMaster->m_ipDev   = pEcDevice;


		memcpy(&pMaster->m_macDest , &pDati[iPointer], sizeof( ETHERNET_ADDRESS ));
 		iPointer += sizeof( ETHERNET_ADDRESS  );
		memcpy(&pMaster->m_macSrc , &pDati[iPointer], sizeof( ETHERNET_ADDRESS ));

		memcpy( &pMaster->m_macSrc, &cMacAddress, sizeof(ETHERNET_ADDRESS) );
		
		iPointer += sizeof( ETHERNET_ADDRESS  );
		memcpy(&pMaster->m_logAddressMBoxStates  , &pDati[iPointer], sizeof( unsigned long  ));
 		iPointer += sizeof(unsigned long );
		memcpy(&pMaster->m_sizeAddressMBoxStates  , &pDati[iPointer], sizeof( unsigned long  ));
 		iPointer += sizeof(unsigned long );
		memcpy(&pMaster->m_szName  , &pDati[iPointer], ECAT_DEVICE_NAMESIZE+1 );
 		iPointer += ECAT_DEVICE_NAMESIZE+1;

		memcpy(&pMaster->m_nInitCmds  , &pDati[iPointer], sizeof( unsigned long  ));
 		iPointer += sizeof(unsigned long );


	//<3------

		pEcDevice->m_pEcMaster = pMaster;

		//memset( &pEcDevice->m_pEcMaster->m_listFree ,0 ,sizeof(LIST_ENTRY_AB));
		//memset( &pEcDevice->m_pEcMaster->m_listPend ,0 ,sizeof(LIST_ENTRY_AB));
		//memset( &pEcDevice->m_pEcMaster->m_mailbox ,0 ,sizeof(EcMailbox));


		pMaster->m_ipDev   = pEcDevice;

                               
		if ( pMaster->m_maxSlaves )
		{
  
	 		pMaster->m_ppEcSlave			= new EcSlave*[pMaster->m_nEcSlave];
	 		memset(pMaster->m_ppEcSlave, 0, pMaster->m_nEcSlave*sizeof( EcSlave*));
	 		pMaster->m_ppEcMbSlave			= new EcMbSlave*[pMaster->m_nEcMbSlave];
	  		memset(pMaster->m_ppEcMbSlave, 0, pMaster->m_nEcMbSlave*sizeof(EcMbSlave*));


	 		pMaster->m_pEcSlave= new CHashTableDyn<unsigned short, EcSlave*>(pMaster->m_nEcSlave);


			pMaster->m_currState			= EC_MASTER_STATE_INIT;
			pMaster->m_reqState				= pMaster->m_currState;
			pMaster->m_maxAsyncFrameSize	= ETHERNET_MAX_FRAME_LEN;
			
  			InitializeListHead(&pMaster->m_listFree);
			InitializeListHead(&pMaster->m_listPend);
			
			pMaster->m_pPend = NULL;
 
			pMaster->m_pInfoBuffer = new ECAT_SLAVEFRAME_INFO[MAX_SLAVEFRM];
			PECAT_SLAVEFRAME_INFO pData = pMaster->m_pInfoBuffer;
			if ( pData )
			{
 				memset ( pData, 0, MAX_SLAVEFRM*sizeof(ECAT_SLAVEFRAME_INFO) );

				for ( int i=0; i < MAX_SLAVEFRM; i++ )
				{
					pData->idx							= EC_HEAD_IDX_SLAVECMD + i;
					pData->frame.Ether.Destination		= pMaster->m_macDest;
					pData->frame.Ether.Source			= pMaster->m_macSrc;
					pData->frame.Ether.FrameType		= ETHERNET_FRAME_TYPE_BKHF_SW;
					pData->frame.E88A4.Type				= ETYPE_88A4_TYPE_ECAT;
					pData->frame.E88A4.Length			= 0;
					pData->frame.E88A4.Reserved		= 0;

					InsertTailListAB(&pMaster->m_listFree, (PLIST_ENTRY_AB)pData);
					pData++;
				}
			}

			pMaster->m_ppEcPortMBoxState	= NULL;
			if ( pMaster->m_sizeAddressMBoxStates > 0 )
			{	//master is configured to check the state of the mailbox
				pMaster->m_ppEcPortMBoxState	= new EcMbSlave*[pMaster->m_sizeAddressMBoxStates*8];
				memset(pMaster->m_ppEcPortMBoxState, 0, pMaster->m_sizeAddressMBoxStates*8*sizeof(EcMbSlave*));
			}
			
			pMaster->m_cInitCmds		= INITCMD_INACTIVE;
			pMaster->m_pInitCmds		= (PEcInitCmdDesc)new unsigned char[pMaster->m_initcmdLen];

			

			pMaster->m_ppInitCmds	= new PEcInitCmdDesc[pMaster->m_nInitCmds];
			if ( pMaster->m_pInitCmds && pMaster->m_ppInitCmds )
			{

	//------4>	
				memcpy(  pMaster->m_pInitCmds, &pDati[iPointer],  pMaster->m_initcmdLen);
				iPointer +=  pMaster->m_initcmdLen;

				memcpy( &pMaster->m_pRefClock, &pDati[iPointer], sizeof(void * ) );
				iPointer +=  sizeof(void * );
				memcpy( &pMaster->m_dcSyncMode, &pDati[iPointer], sizeof(unsigned long ) );



				iPointer +=  sizeof(unsigned long );
				memcpy( &pMaster->m_nDcTimingCounter, &pDati[iPointer], sizeof(unsigned long ) );



				iPointer +=  sizeof(unsigned long );
				memcpy( &pMaster->m_nDcTimingSendArmw, &pDati[iPointer], sizeof(unsigned long ) );



				iPointer +=  sizeof(unsigned long );
	//<4------

				PEcInitCmdDesc	pInitCmds = pMaster->m_pInitCmds;
				for ( unsigned long i=0; i < pMaster->m_nInitCmds; i++ )
				{
					pMaster->m_ppInitCmds[i] = pInitCmds;
					pInitCmds = NEXT_EcInitCmdDesc(pInitCmds);
				}
			}
			else
				pMaster->m_nInitCmds		= 0;	

		
			
			pMaster->m_nEcCycInfo		= 0;
			pMaster->m_maxEcCycInfo		= ETHERNETRTMP_MAXECFRAMES;
			pMaster->m_ppEcCycInfo		= new PEcCycInfo[pMaster->m_maxEcCycInfo];
			memset(pMaster->m_ppEcCycInfo, 0, pMaster->m_maxEcCycInfo*sizeof(PEcCycInfo));

			pMaster->m_pInfoBuffer->timeout	= 0;

			////////// TimeoutHelper /////////////////
			if ( pMaster->m_pInfoBuffer->timeout)
			{																
				delete pMaster->m_pInfoBuffer->timeout ;
				pMaster->m_pInfoBuffer->timeout = NULL;
			}	

			pMaster->m_pInfoBuffer->timeout = new TimeoutHelper;
			Start( pMaster->m_pInfoBuffer->timeout , 0);



			pMaster->m_nEcMbSlave = 0;
 
			for( unsigned short i=0; i<pMaster->m_nEcSlave; i++)
			{																
				DefinisciCopiaSlave( pEcDevice, pDati, i , &iPointer); 
			}
			
			if ( pMaster->m_tCyclicSend )
			{																
				delete pMaster->m_tCyclicSend ;
				pMaster->m_tCyclicSend = NULL;
			}	
			pMaster->m_tCyclicSend = new TimeoutHelper;
			Start( pMaster->m_tCyclicSend , 0);
			
		////////// TimeoutHelper /////////////////
			if ( pMaster->m_tInitCmds )
			{																
				delete pMaster->m_tInitCmds ;
				pMaster->m_tInitCmds = NULL;
			}	
			pMaster->m_tInitCmds = new TimeoutHelper;
			Start( pMaster->m_tInitCmds , 0);	


			if ( pMaster->m_nEcCycInfo < pMaster->m_maxEcCycInfo )
			{


				unsigned short cmdCnt;	
				unsigned long  iT;
				unsigned long  size;
				unsigned long  iCycInfo; 


	//------9>	
				memcpy( &iCycInfo , &pDati[iPointer], sizeof(unsigned long));
				iPointer +=sizeof(unsigned long);
	//<9------


				for ( iT=0 ; iT < iCycInfo   ; iT++)
				{
					pMaster->m_nEcCycInfo = iT;

	//-----10>	
					memcpy( &cmdCnt , &pDati[iPointer], sizeof(unsigned short));
					iPointer +=sizeof(unsigned short);
	//<10-----

					size = sizeof(EcCycInfo) + cmdCnt*sizeof(EcCmdInfo);

					pMaster->m_ppEcCycInfo[iT] = (PEcCycInfo)new unsigned char[size];
					if ( pMaster->m_ppEcCycInfo[iT] )
					{
						memset(pMaster->m_ppEcCycInfo[iT], 0, size);

						//normally this is the broadcast ethernet address.
	//-----11>	
						memcpy( &pMaster->m_ppEcCycInfo[iT]->e88A4Frame.Ether.Destination , &pDati[iPointer], sizeof(ETHERNET_ADDRESS));
						iPointer +=sizeof(ETHERNET_ADDRESS);
						memcpy( &pMaster->m_ppEcCycInfo[iT]->vlanInfo , &pDati[iPointer], sizeof(ETYPE_VLAN_HEADER));
						iPointer +=sizeof(ETYPE_VLAN_HEADER);
						memcpy( &pMaster->m_ppEcCycInfo[iT]->state , &pDati[iPointer], sizeof(unsigned short));
						iPointer +=sizeof(unsigned short);
						memcpy( &pMaster->m_ppEcCycInfo[iT]->cdlNo , &pDati[iPointer], sizeof(unsigned short));
						iPointer +=sizeof(unsigned short);
						memcpy( &pMaster->m_ppEcCycInfo[iT]->imageOffs[0] , &pDati[iPointer], sizeof(unsigned short));
						iPointer +=sizeof(unsigned short);
						memcpy( &pMaster->m_ppEcCycInfo[iT]->imageOffs[1] , &pDati[iPointer], sizeof(unsigned short));
						iPointer +=sizeof(unsigned short);
	//<11-----

											
						pMaster->m_ppEcCycInfo[iT]->e88A4Frame.Ether.Source		= pMaster->m_macSrc;
						pMaster->m_ppEcCycInfo[iT]->e88A4Frame.Ether.FrameType	= ETHERNET_FRAME_TYPE_BKHF_SW;
						pMaster->m_ppEcCycInfo[iT]->e88A4Frame.E88A4.Type		= ETYPE_88A4_TYPE_ECAT;
						pMaster->m_ppEcCycInfo[iT]->e88A4Frame.E88A4.Length		= 0;
						
						/// Andrea
						pMaster->m_ppEcCycInfo[iT]->cmdCnt						= cmdCnt;
	 						
						PEcCmdInfo pVarInfo ;

						for ( unsigned short i=0; i < pMaster->m_ppEcCycInfo[iT]->cmdCnt; i++ )
						{  
						
 							pVarInfo				= (&((PEcCmdInfo)(((PEcCycInfo)pMaster->m_ppEcCycInfo[iT])+1))[i]);
							pVarInfo->pCycInputs	= NULL;
							pVarInfo->pCycOutputs	= NULL;
	//-----12>	
							memcpy( &pVarInfo->imageOffs[0] , &pDati[iPointer], sizeof(unsigned short));
							iPointer +=sizeof(unsigned short);
							memcpy( &pVarInfo->imageOffs[1] , &pDati[iPointer], sizeof(unsigned short));
							iPointer +=sizeof(unsigned short);
							memcpy( &pVarInfo->imageSize[0] , &pDati[iPointer], sizeof(unsigned short));
							iPointer +=sizeof(unsigned short);
							memcpy( &pVarInfo->imageSize[1] , &pDati[iPointer], sizeof(unsigned short));
							iPointer +=sizeof(unsigned short);
							memcpy( &pVarInfo->cmdHeader , &pDati[iPointer], sizeof(ETYPE_EC_HEADER));
							iPointer +=sizeof(ETYPE_EC_HEADER);
							memcpy( &pVarInfo->cntSend , &pDati[iPointer], sizeof(unsigned short));
							iPointer +=sizeof(unsigned short);
							memcpy( &pVarInfo->cntRecv , &pDati[iPointer], sizeof(unsigned short));
							iPointer +=sizeof(unsigned short);
							memcpy( &pVarInfo->cmdSize , &pDati[iPointer], sizeof(unsigned short));
							iPointer +=sizeof(unsigned short);
							memcpy( &pVarInfo->copyInputs , &pDati[iPointer], sizeof(unsigned short));
							iPointer +=sizeof(unsigned short);
							memcpy( &pVarInfo->copyOutputs , &pDati[iPointer], sizeof(unsigned short));
							iPointer +=sizeof(unsigned short);
							memcpy( &pVarInfo->mboxState , &pDati[iPointer], sizeof(unsigned short));
							iPointer +=sizeof(unsigned short);
							memcpy( &pVarInfo->cyclic , &pDati[iPointer], sizeof(unsigned short));
							iPointer +=sizeof(unsigned short);
							memcpy( &pVarInfo->state , &pDati[iPointer], sizeof(unsigned short));
							iPointer +=sizeof(unsigned short);
	//<12-----

							pVarInfo->cmdHeader.idx = (unsigned char)pMaster->m_nEcCycInfo+EC_HEAD_IDX_CYCLIC_START;
							
							pMaster->m_ppEcCycInfo[iT]->e88A4Frame.E88A4.Length += ETYPE_EC_CMD_LEN(&pVarInfo->cmdHeader);
							//add cyclic command to hash table
							/*if ( !pMaster->m_mapEcCmdInfo.Add(( pVarInfo->cmdHeader.cmdIdx + (pVarInfo->cmdHeader.len << 16)), pVarInfo) )
							{
								delete[] pMaster->m_ppEcCycInfo[iT];
							}*/				
						}
					}
					pMaster->m_nEcCycInfo++	;				
				}
				iPointer = iPointer;
			}
		}
	}
	else
	{  
		pEcDevice->m_pFifoE88A4		= new CFiFoList<PETHERNET_88A4_FRAME, ETHERNETRTMP_MAXE88A4FIFO>();	
		pEcDevice->m_pFifoEcFrame	= new CFiFoList<PEcUserFrameInfo, ETHERNETRTMP_MAXECUSERFIFO>();
		iTuttoEtherCAT |= 1;
	}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


	return 1;

}





unsigned char  CancellaEtherCATSchedaRTL8139() 
{
	iTuttoEtherCAT                = 0;
 	ChiudiEtherCAT( );
	iTuttoEtherCAT                = 0;
 	iCampionaturaDaEthernet       = 0;
	iSiScambio                    = 0;

	return 1;
}



/////////////////////////////////////////////////////////////////////////////////////////

unsigned char InizializzazioneEtherCATSchedaRTL8139(  void )
{
	if ( pEcDevice )
	{

		void * pMaster = GetMasterPointerExecute(pEcDevice);

		if( pMaster )
		{
			CaricaEcSyncExecute(  
					   pMaster 
					  );
		}
	 	pEcDevice->m_pEcNpfDevice->m_pEcDevice = pEcDevice;
		if( OpenEcDevice( pEcDevice ) )
		{			
			iTuttoEtherCAT |= 2;
		}
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////




void  EtherCAT_CloseDevice(  )
{
	return ;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


unsigned short ControllaSincronismo( void * pData,void * pDataIni, unsigned long nData )
{
	if ( nData < ETHERNET_FRAME_LEN )
		return 0;

	PETHERNET_FRAME		pFrame		= (PETHERNET_FRAME)pData;
	unsigned short *	pFrameType	= FRAMETYPE_PTR(pFrame);
	unsigned char *	    pFrameTypeB; 
	
	unsigned short  	iTipoFrame;
	unsigned short  	iMsgSync;

	pFrameTypeB = 	(unsigned char *)pFrameType;

	{
		memcpy( &iTipoFrame, pFrameTypeB ,   2);
		memcpy( &iMsgSync, &pFrameTypeB[2] , 2);
		if (    ( iTipoFrame == 0x55FB )
		     && ( iMsgSync   == 0xBAAB ) )
		{
			iSiScambio = 1;
			return 1;
		}
		else


		{

			unsigned long iMacSorgAdd[2];

			memcpy( (unsigned char *)&iMacSorgAdd, pDataIni , 8);
			if (     (  iMacSorgAdd[0] == 0x04030201 )
				   &&(  iMacSorgAdd[1] == 0x08070605 ) )
			{
				iSiScambio = 1;
				return 1;
			}
		}
	}
	return 0;
}



/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////



void GestioneRicezioneSchedaRTL8139(  )
{
        Ricevi();
        //while(Ricevi());
	if ( iTuttoEtherCAT == 70)
	{
		if ( pEcDevice )
		{ 
                        /*
			if ( iAggancia == 1 )
			{ 
				if ( iRinegozia == 1 )
				{ 
					Ttune				= 0;
					iManda				= 1;
					iAggancia           = 0;
					iFaseEtherCAT		= 1;
					iCuntaFaseEtherCAT	= 0;
					iCollegatoEtherCAT  = 0;
				} 
				else
				{ 
					return;
				} 
			} 
			if ( iManda == 2  )
			{ 
				return;

			} 
			if(   ( iRinegozia == 2 )
				||( iRinegozia == 3 ) )
			{ 
				return;

			} 
                        */
			{
                                if( iFaseEtherCAT > 0)
				{ 
					if( iFaseEtherCAT == 1)
					{ 
						if ( iCollegatoEtherCAT == 0 )
						{ 
							iCuntaFaseEtherCAT++;
							if( iCuntaFaseEtherCAT >= 0 )
							{ 
								void * pMaster = GetMasterPointerExecute(pEcDevice);

								if( pMaster )
								{
						 			RequestStateMasterExecute( pMaster, DEVICE_STATE_INIT );		
									iFaseEtherCAT		= 2;
									iCuntaFaseEtherCAT	= 0;
								} 
							} 
						} 
					} 
					if( iFaseEtherCAT == 2)
					{ 
                                                iCuntaFaseEtherCAT++;
						iMinchiaEtherCAT    = 50;
						if( iCuntaFaseEtherCAT >= 20)
						{ 
							void * pMaster = GetMasterPointerExecute(pEcDevice);

							if( pMaster )
							{
								RequestStateMasterExecute( pMaster, DEVICE_STATE_PREOP );		
								iFaseEtherCAT		= 3;
								iCuntaFaseEtherCAT	= 0;
							} 
						} 
					} 
					if( iFaseEtherCAT == 3)
					{ 
						iCuntaFaseEtherCAT++;  
						if( iCuntaFaseEtherCAT >= iMinchiaEtherCAT  )   // era 300 2000
						{ 
							void * pMaster = GetMasterPointerExecute(pEcDevice);

							if( pMaster )
							{
								RequestStateMasterExecute( pMaster, DEVICE_STATE_SAFEOP );		
								iFaseEtherCAT		= 4;
								iCuntaFaseEtherCAT	= 0;
							} 
						} 
					} 
					if( iFaseEtherCAT == 4)
					{ 
						iCuntaFaseEtherCAT++;
						if( iCuntaFaseEtherCAT >= 20)
						{ 
							void * pMaster = GetMasterPointerExecute(pEcDevice);

							if( pMaster )
							{
								RequestStateMasterExecute( pMaster, DEVICE_STATE_OP );		
								iFaseEtherCAT		= 5;
								iCuntaFaseEtherCAT	= 0;
							} 
						} 
					} 
					if( iFaseEtherCAT == 5)
					{ 
						iCuntaFaseEtherCAT++;
						if( iCuntaFaseEtherCAT >= 20)
						{ 
							iFaseEtherCAT		= 0;
							iCuntaFaseEtherCAT	= 0;
						} 
					} 
				}
				{
					OnTimerEcDeviceExecute ( pEcDevice );
				}
				if ( iFaseEtherCAT == 0)	
				{ 
					int stao;

					void * pMaster = GetMasterPointerExecute(pEcDevice);
					if ( pMaster )
					{ 
						stao =  0;
						{
							switch ( ((PEcMaster)pMaster)->m_currState )
							{
								case EC_MASTER_STATE_INIT:
								case EC_MASTER_STATE_WAIT_SLAVE_I_P:
								case EC_MASTER_STATE_WAIT_SLAVE_P_I:
								case EC_MASTER_STATE_WAIT_SLAVE_S_I:
								case EC_MASTER_STATE_WAIT_SLAVE_O_I:
									stao = DEVICE_STATE_INIT;

								case EC_MASTER_STATE_PREOP:
								case EC_MASTER_STATE_WAIT_SLAVE_P_S:
								case EC_MASTER_STATE_WAIT_SLAVE_S_P:
								case EC_MASTER_STATE_WAIT_SLAVE_O_P:
									stao = DEVICE_STATE_PREOP;

								case EC_MASTER_STATE_SAFEOP:
								case EC_MASTER_STATE_WAIT_SLAVE_S_O:
								case EC_MASTER_STATE_WAIT_SLAVE_O_S:
									stao = DEVICE_STATE_SAFEOP;

								case EC_MASTER_STATE_OP:
									stao = DEVICE_STATE_OP;
							}
						}
						if ( stao == DEVICE_STATE_OP ) 
						{ 
  							iEtherCATOperationale		= 1;
							iEtherCATConnesso		= 1;
						}
						else
						{ 
							iEtherCATOperationale		= 0;
							iFaseEtherCAT		= 1;
							iCuntaFaseEtherCAT	= 0;
							iEtherCATConnesso		= 0;
						}
 						iCollegatoEtherCAT++;
						if ( iCollegatoEtherCAT > 500 )
						{ 
							iFaseEtherCAT		= 1;
							iCuntaFaseEtherCAT	= 0;
							iEtherCATConnesso		= 0;
							iEtherCATErrore ++;
						}
					}
				}
			}

			{ 
				unsigned long    nData = 0;
				unsigned char *  pData = NULL;

				nData = ProcessDataSizeEcDeviceExecute ( pEcDevice,
														 VG_IN
													   );
				pData = ProcessDataPtrEcDeviceExecute ( pEcDevice,
														VG_IN,
														0,
														nData
													  );
				{
					//RimappaCaricaInput( pData,  nData);
				}
			}
		}
	}
}


void GestioneTrasmissioneSchedaRTL8139( )
{
	unsigned long    nData = 0;
	unsigned char *  pData = NULL;


	if ( iTuttoEtherCAT == 70)
	{
                /*
		if(  iManda == 2 )
		{ 
			return;
		} 
		if(  iRinegozia  != 1 )
		{ 
			return;
		} 
 		if(     
				(  iFaseEtherCAT      == 1 ) 
			&&	(  iCuntaFaseEtherCAT == 0 )
		  )
		{ 
		//	return;
		} 
		if ( iAggancia == 1 )
		{ 
			return;
		} 
                */
		if ( pEcDevice )
		{
			nData = ProcessDataSizeEcDeviceExecute ( pEcDevice,
													 VG_OUT
												   );			
			pData = ProcessDataPtrEcDeviceExecute ( pEcDevice,
													VG_OUT,
													0,
													nData
												  );			
			// copia i dati degli output da inviare
			//RimappaCaricaOutput( pData,  nData );

			GetIoStateEcDeviceExecute( pEcDevice );
			StartIoEcDeviceExecute( pEcDevice );									
		}
	}
}

void WriteBufferInterna(int iStart, int iLen, char * pLocalBuffer)
{
	unsigned long    nData = 0;
	unsigned char *  pData = NULL;

	if ( iTuttoEtherCAT == 70)
	{
		if ( pEcDevice )
		{
			nData = ProcessDataSizeEcDeviceExecute ( pEcDevice,
													 VG_OUT
												   );			
			pData = ProcessDataPtrEcDeviceExecute ( pEcDevice,
													VG_OUT,
													0,
													nData
												  );
			
			memcpy(pData+iStart, pLocalBuffer, iLen);
		}
	}
}

void ReadBufferInterna(int iStart, int iLen, char * pLocalBuffer)
{
	unsigned long    nData = 0;
	unsigned char *  pData = NULL;

	if ( iTuttoEtherCAT == 70)
	{
		if ( pEcDevice )
		{
			nData = ProcessDataSizeEcDeviceExecute ( pEcDevice,
													 VG_IN
												   );
			pData = ProcessDataPtrEcDeviceExecute ( pEcDevice,
													VG_IN,
													0,
													nData
												  );

			memcpy(pLocalBuffer, pData+iStart, iLen);
		}
	}
}
