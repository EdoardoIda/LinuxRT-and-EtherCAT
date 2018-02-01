#include <SPI.h>         // needed for Ethernet library communication with the W5100 (Arduino ver>0018)

#include "AxesBrainEcInterface.h"
#include "Strutture.h"
#include "AxesBrainEcNpfDevice.h"
#include "AxesBrainEcDevice.h"

int64_t  EcatSysTime = 0;

extern unsigned int		iPrecedente;	 
extern bool				SysTimeRead;
extern int				iMinchiaEtherCAT;
extern int				Ttune;	
extern int				LenEcatSysTime;	
extern short                     iChi;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////               MASTER            ///////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

int OpenEcMaster( PEcMaster pEcMaster ) 
{
        unsigned long i;
  
	//initialize inputs and outputs of the cyclic commands
	for ( i=0; i < pEcMaster->m_nEcCycInfo; i++ )
	{
		//set the source MAC address of the cyclic frames
		pEcMaster->m_ppEcCycInfo[i]->e88A4Frame.Ether.Source = pEcMaster->m_macSrc;				
		for ( unsigned long j=0; j < pEcMaster->m_ppEcCycInfo[i]->cmdCnt; j++ )
		{
			PEcCmdInfo pEcCmd = EcCycCmdInfo(pEcMaster->m_ppEcCycInfo[i], j);

			pEcCmd->pCycInputs	= ProcessDataPtr( (PEcDevice)pEcMaster->m_ipDev, VG_IN, pEcCmd->imageOffs[0]+sizeof(ETYPE_EC_HEADER), pEcCmd->imageSize[0]);
			pEcCmd->pCycOutputs	= ProcessDataPtr( (PEcDevice)pEcMaster->m_ipDev, VG_OUT, pEcCmd->imageOffs[1]+sizeof(ETYPE_EC_HEADER), pEcCmd->imageSize[1]);

			if ( pEcCmd->pCycInputs == NULL || pEcCmd->pCycOutputs == NULL )
			{
				return EC_E_INVALIDDATA;
			}
		}
	}	
	pEcMaster->m_reqState = pEcMaster->m_ioOpenStateInit ? DEVICE_STATE_INIT : DEVICE_STATE_PREOP;


	if ( pEcMaster->m_dcSyncMode )
	{
		for ( i=0; i < pEcMaster->m_nEcSlave; i++ )
		{	// set neighbor pointers


			EcSlave* pI = pEcMaster->m_ppEcSlave[i];
			EcSlave* pP;

			if ( pEcMaster->m_pEcSlave->Lookup(pI->m_prevPhysAddr, pP) )
			{
				EcSlave* pD = pP;
				
				pI->m_pEcSlaves[EC_A]			= pD;
				pD->m_pEcSlaves[pI->m_prevPort]	= pI;
			}
		}
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
///\brief This method is called cyclically by CEcDevice::OnTimer.
int	OnTimerEcMaster( PEcMaster pEcMaster, unsigned long nTickNow )
{
	PEcDevice    pEcDevice;

	pEcDevice =  (PEcDevice)pEcMaster->m_ipDev;

	CheckAsyncFrameEcMaster(pEcMaster);

	if ( pEcMaster->m_currState != EC_MASTER_STATE_INIT )
	{				
		// call OnTimer of mailbox slaves
		EcMbSlave* pMbSlave;

		//call OnTimer once for all Mailbox slaves 
		while ( pEcMaster->m_listProcessOnTimer.Remove(pMbSlave) )
			OnTimerEcMbSlave(pMbSlave , 0); 

		//call the OnTimer method of each mailbox slave on every MBOX_SLAVE_ONTIMER_DIV tick
		//here the still outstanding mailbox commands are sent 
		for ( unsigned long i=(nTickNow%MBOX_SLAVE_ONTIMER_DIV); i < pEcMaster->m_nEcMbSlave; i+=MBOX_SLAVE_ONTIMER_DIV )
			OnTimerEcMbSlave( pEcMaster->m_ppEcMbSlave[i] , nTickNow/MBOX_SLAVE_ONTIMER_DIV );
	}	

	// timeout and retry monitoring of pending ethercat commands
	PECAT_SLAVEFRAME_INFO pInfo = NULL;
	
	if ( !IsListEmpty(&pEcMaster->m_listPend) )
	{	// pending frame - check for timeout
  		pInfo = (PECAT_SLAVEFRAME_INFO)GetListHeadEntry(&pEcMaster->m_listPend);
  
                iChi = 1;
              	if ( IsElapsed(pInfo->timeout) )
		{	//timeout has elapsed remove list entry
			RemoveEntryListAB((PLIST_ENTRY_AB)pInfo);
		}
		else
			pInfo = NULL;
	}	

	if ( pInfo && pInfo->retry > 0)
	{	// a pInfo value != 0 indicates that the timeout has elapsed for the pending frame.
		// if the retry counter is greater 0 resend frame 		
		
		//decrement retry counter
		pInfo->retry--;
		//restart timer
		Start(pInfo->timeout, TIMEOUT_RELOAD);
		unsigned long nFrame = SIZEOF_88A4_FRAME(&pInfo->frame);
		PETHERNET_88A4_FRAME pFrame = (PETHERNET_88A4_FRAME)FrameAllocEcDevice(pEcDevice, nFrame);
		if ( pFrame )
		{
			memcpy(pFrame, &pInfo->frame, nFrame);
			//add the frame to the queue of frames to be sent to the ethernet adapter; 
			if ( QueueE88A4CmdEcDevice(pEcDevice, pFrame) )
			{	//add info of this frame to the pending list again			
				InsertTailListAB(&pEcMaster->m_listPend, (PLIST_ENTRY_AB)pInfo);
				pInfo = NULL;				
			}
			else
			{	// failed to queue frame -> free frame
				FrameFreeEcDevice(pEcDevice,pFrame);
			}
		}
	}
				
	if ( pInfo )
	{	// no retries left -> inform sources
		for ( unsigned int i=0; i < pInfo->nInfo; i++ )
		{
			if ( pInfo->cmdInfo[i].pSlave )
				//frame originated from a CEcSlave object
				EcatCmdResEcSlave((PEcSlave)pInfo->cmdInfo[i].pSlave, ECERR_DEVICE_TIMEOUT, pInfo->cmdInfo[i].invokeId, NULL,0);
			else
				//frame originated from this object(CEcMaster)
				EcatCmdResEcMaster(pEcMaster, ECERR_DEVICE_TIMEOUT, pInfo->cmdInfo[i].invokeId, NULL);
		}

		// remove old frame and cmds
		pInfo->frame.E88A4.Length	= 0;
		pInfo->pLastHead				= NULL;
		pInfo->nInfo					= 0;
				
		InsertTailListAB(&pEcMaster->m_listFree, (PLIST_ENTRY_AB)pInfo);		
	}

	// flush waiting ethercat commands contained in the pending list m_pPend
	EcatCmdFlushEcMaster( pEcMaster);
	
	return 1;
}





void SetSrcMacAddressEcMaster( PEcMaster pEcMaster, ETHERNET_ADDRESS macAdapter )		
{
	pEcMaster->m_macSrc = macAdapter;
	return ;
}



///////////////////////////////////////////////////////////////////////////////
///\brief Adds the mailbox slave pSlave to the list of slaves whose OnTimer method should be called once in CEcMaser->OnTimer
void	AddToOnTimerListEcMaster(PEcMaster pEcMaster, PEcMbSlave pMbSlave)
{
	pEcMaster->m_listProcessOnTimer.Add(pMbSlave);
}


unsigned long CalcDcRefClockDelayEcMaster(PEcMaster pEcMaster, EcSlave* pSlave, unsigned long delay, EcSlave* pPrev)
{
	if ( pPrev == pSlave->m_pEcSlaves[EC_A] )
	{	
		//add line delay between previous slave port and port A of pSlave
		delay += pSlave->m_nDcLineDelay[EC_A];

		delay += pSlave->m_nDcPortDelayR[EC_A];

		delay += pSlave->m_nDcExecDelay;

		pSlave->m_nDcRefClockDelay928 = delay;	

		//	pSlave->m_nDcLineDelay[EC_A], pSlave->m_nDcLineDelay[EC_B], pSlave->m_nDcLineDelay[EC_C], pSlave->m_nDcLineDelay[EC_D], pSlave->m_szName);

		if ( pSlave->m_pEcSlaves[EC_D] )
		{
			delay += pSlave->m_nDcPortDelayT[EC_D];
			//recursively calculate and add delay of all slaves connected to port D 
			delay = CalcDcRefClockDelayEcMaster(pEcMaster, (EcSlave*)pSlave->m_pEcSlaves[EC_D], delay, pSlave);
			delay += pSlave->m_nDcLineDelay[EC_D];
			delay += pSlave->m_nDcPortDelayR[EC_D];
		}
		if ( pSlave->m_pEcSlaves[EC_B] )
		{
			delay += pSlave->m_nDcPortDelayT[EC_B];
			//recursively calculate and add delay of all slaves connected to port B 
			delay = CalcDcRefClockDelayEcMaster(pEcMaster, (EcSlave*)pSlave->m_pEcSlaves[EC_B], delay, pSlave);
			delay += pSlave->m_nDcLineDelay[EC_B];
			delay += pSlave->m_nDcPortDelayR[EC_B];
		}
		if ( pSlave->m_pEcSlaves[EC_C] )
		{
			delay += pSlave->m_nDcPortDelayT[EC_C];
			//recursively calculate and add delay of all slaves connected to port C 
			delay = CalcDcRefClockDelayEcMaster(pEcMaster, (EcSlave*)pSlave->m_pEcSlaves[EC_C], delay, pSlave);
			delay += pSlave->m_nDcLineDelay[EC_C];
			delay += pSlave->m_nDcPortDelayR[EC_C];
		}	

		delay += pSlave->m_nDcPortDelayT[EC_A];

	}
	else 
	{	// backwards
		if ( pPrev == pSlave->m_pEcSlaves[EC_D] && pSlave->m_pEcSlaves[EC_B] )
		{
			delay += pSlave->m_nDcPortDelayT[EC_B];
			//recursively calculate and add delay of all slaves connected to port B 
			delay = CalcDcRefClockDelayEcMaster(pEcMaster, (EcSlave*)pSlave->m_pEcSlaves[EC_B], delay, pSlave);
			delay += pSlave->m_nDcLineDelay[EC_B];
			delay += pSlave->m_nDcPortDelayR[EC_B];
		}
		if ( (pPrev == pSlave->m_pEcSlaves[EC_D] || pPrev == pSlave->m_pEcSlaves[EC_B]) && pSlave->m_pEcSlaves[EC_C] )
		{
			delay += pSlave->m_nDcPortDelayT[EC_C];
			//recursively calculate and add delay of all slaves connected to port C 
			delay = CalcDcRefClockDelayEcMaster(pEcMaster, (EcSlave*)pSlave->m_pEcSlaves[EC_C], delay, pSlave);
			delay += pSlave->m_nDcLineDelay[EC_C];
			delay += pSlave->m_nDcPortDelayR[EC_C];
		}
		
		if ( pSlave->m_pEcSlaves[EC_A] )
		{
			delay += pSlave->m_nDcPortDelayT[EC_A];
			// recursively search for port C branches before ref clock
			delay = CalcDcRefClockDelayEcMaster(pEcMaster, (EcSlave*)pSlave->m_pEcSlaves[EC_A], delay, pSlave);
		}
	}

	//	pSlave->m_nDcLineDelay[EC_A], pSlave->m_nDcLineDelay[EC_B], pSlave->m_nDcLineDelay[EC_C], pSlave->m_nDcLineDelay[EC_D], pSlave->m_szName );

	return delay;
}

///////////////////////////////////////////////////////////////////////////////
void	CalcDcTimingsEcMaster(PEcMaster pEcMaster)
{
	EcSlave* pRefClock = pEcMaster->m_pRefClock;

	ULONGLONG sysTime=0;
	int i;
	for ( i=0; i < (int)pEcMaster->m_nEcSlave; i++ )
		ExecDcTimingEcSlave(pEcMaster->m_ppEcSlave[i], EC_DC_TIMING_CALC_TIMING);

	//reset line delay and reference clock delay
	for ( i=0; i < (int)pEcMaster->m_nEcSlave; i++ )
	{
		// reset delay values
		memset(pEcMaster->m_ppEcSlave[i]->m_nDcLineDelay, 0, sizeof(pEcMaster->m_ppEcSlave[i]->m_nDcLineDelay));
		pEcMaster->m_ppEcSlave[i]->m_nDcRefClockDelay928 = 0;
	}

	// calculate line delays
	for ( i=0; i < (int)pEcMaster->m_nEcSlave; i++ )
	{
		EcSlave* pI = pEcMaster->m_ppEcSlave[i];
		if ( !pI->m_bDcSupport )
			continue;

		long nDcExecDelay = pI->m_nDcExecDelay;
		for ( int p=1; p < ECAT_ESC_PORTS_MAX; p++ )
		{
			//get the ports in the correct order 
			int q = ECAT_PORT_ORDER[p];
			if ( pI->m_pEcSlaves[q] )
			{	
				//get slave at port q
				EcSlave* pQ = (EcSlave*)pI->m_pEcSlaves[q]; 
								
				long delay = pI->m_nDcPortDiff[q] - nDcExecDelay - pI->m_nDcPortDelayT[q] - pI->m_nDcPortDelayR[q];
				nDcExecDelay = 0;
				
				while ( !pQ->m_bDcSupport && pQ->m_pEcSlaves[EC_B] )
				{	// no DC support -> slave pD has only port A and B!
					delay -= pQ->m_nDcPortDelayR[EC_A];
					delay -= pQ->m_nDcExecDelay;					
					delay -= pQ->m_nDcPortDelayT[EC_B];
					delay -= pQ->m_nDcPortDelayR[EC_B];
					delay -= pQ->m_nDcPortDelayT[EC_A];
					pQ = (EcSlave*)pQ->m_pEcSlaves[EC_B];
				}

				//subtract the time the frames takes from and back to port A of the next slave 
				//-> delay = line delay between port q of and port A of next slave + line delay between port A of nex frame and port q
				delay -= DcCalcDelayAAEcSlave(pQ);

				if ( delay > 0 )
				{
					//divide the line delay symmetrical between forward and back direction
					pI->m_nDcLineDelay[q] = pQ->m_nDcLineDelay[EC_A] = delay/2;
					if ( (delay%2) != 0 )
						pI->m_nDcLineDelay[q]++;
				}
			}
		}
	}
	if ( pRefClock )
	{	// calculate offset from ref clock
		//unsigned long delay = pRefClock->m_nDcExecDelay;
		unsigned long delay = 0;
		if ( pRefClock->m_pEcSlaves[EC_D] )
		{
			delay += pRefClock->m_nDcPortDelayT[EC_D];
			//recursively calculate and add delay of all slaves connected to port D 
			delay = CalcDcRefClockDelayEcMaster(pEcMaster, (EcSlave*)pRefClock->m_pEcSlaves[EC_D], delay, pRefClock);
			delay += pRefClock->m_nDcLineDelay[EC_D];
			delay += pRefClock->m_nDcPortDelayT[EC_D];
		}
		if ( pRefClock->m_pEcSlaves[EC_B] )
		{
			delay += pRefClock->m_nDcPortDelayT[EC_B];
			//recursively calculate and add delay of all slaves connected to port A
			delay = CalcDcRefClockDelayEcMaster(pEcMaster, (EcSlave*)pRefClock->m_pEcSlaves[EC_B], delay, pRefClock);
			delay += pRefClock->m_nDcLineDelay[EC_B];
			delay += pRefClock->m_nDcPortDelayT[EC_B];
		}
		if ( pRefClock->m_pEcSlaves[EC_C] )
		{
			delay += pRefClock->m_nDcPortDelayT[EC_C];
			//recursively calculate and add delay of all slaves connected to port C 
			delay = CalcDcRefClockDelayEcMaster(pEcMaster, (EcSlave*)pRefClock->m_pEcSlaves[EC_C], delay, pRefClock);
			delay += pRefClock->m_nDcLineDelay[EC_C];
			delay += pRefClock->m_nDcPortDelayT[EC_C];
		}

		//	pRefClock->m_nDcLineDelay[EC_A], pRefClock->m_nDcLineDelay[EC_B], pRefClock->m_nDcLineDelay[EC_C], pRefClock->m_nDcLineDelay[EC_D], pRefClock->m_szName);
	}
	/*
	for ( i=0; i < (int)pEcMaster->m_nEcSlave; i++ )
	{
		EcSlave* pI = pEcMaster->m_ppEcSlave[i];
		{
				pI->m_nDcLineDelay[EC_A], pI->m_nDcLineDelay[EC_B], pI->m_nDcLineDelay[EC_C], pI->m_nDcLineDelay[EC_D], pI->m_szName);
		}
	}
	*/
}


///////////////////////////////////////////////////////////////////////////////
///\brief Returns the current state of the EtherCAT master.
unsigned short	GetStateMachineEcMaster(PEcMaster pEcMaster)
{
	switch (pEcMaster->m_currState)
	{
	case EC_MASTER_STATE_INIT:
	case EC_MASTER_STATE_WAIT_SLAVE_I_P:
	case EC_MASTER_STATE_WAIT_SLAVE_P_I:
	case EC_MASTER_STATE_WAIT_SLAVE_S_I:
	case EC_MASTER_STATE_WAIT_SLAVE_O_I:
	case EC_MASTER_STATE_DCLOCKS:
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



///////////////////////////////////////////////////////////////////////////////
///\brief Executes the state machine of the EtherCAT master.
int	StateMachineEcMaster(PEcMaster pEcMaster, int bOnTimer)
{
	int bMasterReady = 0;
	int bSlavesReady = 1;

	//execute state machine of all slaves and
	//check if all slaves are in the requested state (--> bSlaveReady == 1 )
	for ( unsigned int i=0; i < pEcMaster->m_nEcSlave; i++ )
		bSlavesReady &= StateMachineEcSlave(pEcMaster->m_ppEcSlave[i],0);

	unsigned short	stateValue	= 0;
	EC_MASTER_STATE	stateNext	= EC_MASTER_STATE_NONE;

	switch (pEcMaster->m_currState)
	{
	case EC_MASTER_STATE_INIT:
		//current state
		switch ( pEcMaster->m_reqState )
		{
		case DEVICE_STATE_INIT:
			//state already in init -> device ready
			bMasterReady = 1;
			break;
		case DEVICE_STATE_PREOP:
		case DEVICE_STATE_SAFEOP:
		case DEVICE_STATE_OP:			
			//execute the init commands for the transition Init to Pre-Operational, that should
			//be sent before the slave init commands


			stateValue	= ECAT_INITCMD_I_P|ECAT_INITCMD_BEFORE;
			//next state: wait for all slaves to go to Pre-Operational
			stateNext	= EC_MASTER_STATE_WAIT_SLAVE_I_P;
			stateNext	= EC_MASTER_STATE_DCLOCKS;
			pEcMaster->m_eDcState	= EC_DC_TIMING_INIT;
			break;
		}
		break;
	case EC_MASTER_STATE_DCLOCKS:
		if ( bSlavesReady )
		{	// DC initialization and timing calculation
			if ( pEcMaster->m_dcSyncMode != ECAT_DC_MODE_NONE )
			{
				unsigned int i;
				switch (pEcMaster->m_eDcState)
				{
				case EC_DC_TIMING_INIT:


					// initialize all slaves
					for ( i=0; i < pEcMaster->m_nEcSlave; i++ )
						ExecDcTimingEcSlave(pEcMaster->m_ppEcSlave[i], EC_DC_TIMING_INIT);
					pEcMaster->m_eDcState = EC_DC_TIMING_READ_LINK_STATUS;					
					break;
				case EC_DC_TIMING_READ_LINK_STATUS:


					//EC_TRACE(_T("EC_DC_TIMING_READ_LINK_STATUS\n"));
					// begin to evaluate the topology (read link states)
					for ( i=0; i < pEcMaster->m_nEcSlave; i++ )
						ExecDcTimingEcSlave(pEcMaster->m_ppEcSlave[i], EC_DC_TIMING_READ_LINK_STATUS);
					
					pEcMaster->m_eDcState = EC_DC_TIMING_READ_TIMING;	// read timing counter of slave ports
					pEcMaster->m_nDcTimingCounter = 0;
					break;
				case EC_DC_TIMING_READ_TIMING:
					//EC_TRACE(_T("EC_DC_TIMING_READ_TIMING\n"));
					{	// read timing counter of slave ports
						EcatCmdFlushEcMaster(pEcMaster);
						if ( (pEcMaster->m_nDcTimingCounter % 2) == 0 )
						{	// every second cycle%0
							// slaves should latch the start of frame (SOF) time of this frame
							EcatCmdReqEcMaster(pEcMaster, NULL, ECMI_DC_INIT, EC_CMD_TYPE_BWR, 0, ESC_REG_DC_SOF_LATCH_A, sizeof(ULONGLONG), NULL);
							// send the command in a single frame
							EcatCmdFlushEcMaster(pEcMaster);
						}

						if ( (pEcMaster->m_nDcTimingCounter % 2) == 1 )
						{	// every second cycle%1
							// read timings of the slaves 
							for ( i=0; i < pEcMaster->m_nEcSlave; i++ )
								ExecDcTimingEcSlave(pEcMaster->m_ppEcSlave[i], EC_DC_TIMING_READ_TIMING);
							EcatCmdFlushEcMaster(pEcMaster);
						}

						pEcMaster->m_nDcTimingCounter++;

						if ( pEcMaster->m_nDcTimingCounter >= ECAT_DC_READ_TIMING_CNT_START )
							pEcMaster->m_eDcState = EC_DC_TIMING_CALC_TIMING;
					}
					break;
				case EC_DC_TIMING_CALC_TIMING:

					// EC_TRACE(_T("EC_DC_TIMING_CALC_TIMING\n"));
					// calculate timings and topology
					CalcDcTimingsEcMaster(pEcMaster);
					pEcMaster->m_eDcState = EC_DC_TIMING_WRITE_SYSTIME_OFFS;
					break;
				case EC_DC_TIMING_WRITE_SYSTIME_OFFS:
					// write individual systime offs relating to individual local clock
					//EC_TRACE(_T("EC_DC_TIMING_WRITE_SYSTIME_OFFS\n"));
					for ( i=0; i < pEcMaster->m_nEcSlave; i++ )
						ExecDcTimingEcSlave(pEcMaster->m_ppEcSlave[i], EC_DC_TIMING_WRITE_SYSTIME_OFFS);
					pEcMaster->m_eDcState = EC_DC_TIMING_WRITE_CLOCK_DELAY;
					break;
				case EC_DC_TIMING_WRITE_CLOCK_DELAY:
				 	//t.Trace(TraceInfo,"EC_DC_TIMING_WRITE_CLOCK_DELAY\n" );
					//EC_TRACE(_T("EC_DC_TIMING_WRITE_CLOCK_DELAY\n"));
					// write individual clock delay relating to reference clock
					for ( i=0; i < pEcMaster->m_nEcSlave; i++ )
						ExecDcTimingEcSlave(pEcMaster->m_ppEcSlave[i], EC_DC_TIMING_WRITE_CLOCK_DELAY);
					pEcMaster->m_eDcState = EC_DC_TIMING_WRITE_SPEED_START;
					break;
				case EC_DC_TIMING_WRITE_SPEED_START:
				 	//t.Trace(TraceInfo,"EC_DC_TIMING_WRITE_SPEED_START\n" );
					//EC_TRACE(_T("EC_DC_TIMING_WRITE_SPEED_START\n"));
					// initialize speed control registers
					for ( i=0; i < pEcMaster->m_nEcSlave; i++ )
						ExecDcTimingEcSlave(pEcMaster->m_ppEcSlave[i], EC_DC_TIMING_WRITE_SPEED_START);
					pEcMaster->m_eDcState = EC_DC_TIMING_CHECK_REFCLOCK;
					break;
				case EC_DC_TIMING_CHECK_REFCLOCK:
				 	//t.Trace(TraceInfo,"EC_DC_TIMING_CHECK_REFCLOCK\n" );
					//EC_TRACE(_T("EC_DC_TIMING_CHECK_REFCLOCK\n"));
					// check ESC build of slaves before the reference clock (for unsupported AWMR cmd)
					if ( pEcMaster->m_pRefClock )
					{
						EcSlave* pRef = pEcMaster->m_pRefClock;
						for ( i=0; i < pEcMaster->m_nEcSlave; i++ )
						{
							if ( pEcMaster->m_ppEcSlave[i] == pRef )
								break;

							//t.Trace(TraceInfo,"EC_DC_TIMING_CHECK_REFCLOCK\n" );
							ExecDcTimingEcSlave(pEcMaster->m_ppEcSlave[i], EC_DC_TIMING_CHECK_BUILD_ARMW);
						}
						pEcMaster->m_nDcTimingCounter = 0;
						pEcMaster->m_eDcState = EC_DC_TIMING_SEND_CLOCK;
					}
					else
						pEcMaster->m_eDcState = EC_DC_TIMING_DONE;
					break;
				case EC_DC_TIMING_SEND_CLOCK:
				 	//t.Trace(TraceInfo,"EC_DC_TIMING_SEND_CLOCK\n" );
					//EC_TRACE(_T("EC_DC_TIMING_SEND_CLOCK\n"));
					// send ARMW as often as possible to initialize the speed control
//					if ( (m_nDcTimingCounter % 2) == 0 )
					{
						unsigned long	dcTime=0;
//						for ( int f=0; f < 15; f++ )
 						for ( int f=0; f < 4; f++ )
						{
							if ( pEcMaster->m_dcSyncMode == ECAT_DC_MODE_MASTER )
								EcatCmdReqEcMaster(pEcMaster, NULL, ECMI_DC_INIT, EC_CMD_TYPE_ARMW, ((EcSlave *)pEcMaster->m_pRefClock)->m_autoIncAddr, ESC_REG_DC_SYSTIME, sizeof(dcTime), &dcTime);
							else
								EcatCmdReqEcMaster(pEcMaster, NULL, ECMI_DC_INIT, EC_CMD_TYPE_BWR, 0, ESC_REG_DC_SYSTIME, sizeof(dcTime), &dcTime);
								// send the command in a single frame
							EcatCmdFlushEcMaster(pEcMaster);
						}
					}
					pEcMaster->m_nDcTimingCounter++;
					if ( pEcMaster->m_nDcTimingCounter >= pEcMaster->m_nDcTimingSendArmw )
						pEcMaster->m_eDcState = EC_DC_TIMING_DONE;
					break;
				case EC_DC_TIMING_DONE:
					// thats it!
				 	//t.Trace(TraceInfo,"EC_DC_TIMING_DONE\n" );
					pEcMaster->m_eDcState	= EC_DC_TIMING_INIT;
					stateNext	= EC_MASTER_STATE_WAIT_SLAVE_I_P;
					stateValue	= ECAT_INITCMD_DUMMY_VALUE;

					iMinchiaEtherCAT = 0;
					break;
				}
			}
			else
			{
				for ( unsigned int i=0; i < pEcMaster->m_nEcSlave; i++ )
					ExecDcTimingEcSlave(pEcMaster->m_ppEcSlave[i], EC_DC_TIMING_INIT);
				stateNext	= EC_MASTER_STATE_WAIT_SLAVE_I_P;
				stateValue	= ECAT_INITCMD_DUMMY_VALUE;
			}
		}
		break;
	case EC_MASTER_STATE_WAIT_SLAVE_I_P:
		if ( bSlavesReady )
		{	//all slaves are in Pre-Operational			
			//-->execute the init commands for the transition Init to Pre-Operational
			stateValue	= ECAT_INITCMD_I_P;
			//next state : Pre-Operational
			stateNext	= EC_MASTER_STATE_PREOP;
		}
		break;

	case EC_MASTER_STATE_PREOP:
		switch ( pEcMaster->m_reqState )
		{
		case DEVICE_STATE_INIT:
			//execute the init commands for the transition Pre-Operational to Init, that should
			//be sent before the slave init commands
			stateValue	= ECAT_INITCMD_P_I|ECAT_INITCMD_BEFORE;		
			//next state: wait for all slaves to go to Init
			stateNext	= EC_MASTER_STATE_WAIT_SLAVE_P_I;
			break;
		case DEVICE_STATE_PREOP:
			//state already in Pre-Operational -> device ready
			bMasterReady = 1;
			break;
		case DEVICE_STATE_SAFEOP:
		case DEVICE_STATE_OP:
			//execute the init commands for the transition Pre-Operational to Safe-Operational, that should
			//be sent before the slave init commands
			stateValue	= ECAT_INITCMD_P_S|ECAT_INITCMD_BEFORE;
			//next state: wait for all slaves to go to Safe-Operational
			stateNext	= EC_MASTER_STATE_WAIT_SLAVE_P_S;
			break;
		}
		break;
	case EC_MASTER_STATE_WAIT_SLAVE_P_I:
		if ( bSlavesReady )
		{	//all slaves are in Init
			//-->execute the init commands for the transition Pre-Operational to Init
			stateValue	= ECAT_INITCMD_P_I;
			//next state: Init
			stateNext	= EC_MASTER_STATE_INIT;
		}
		break;
	case EC_MASTER_STATE_WAIT_SLAVE_P_S:
		if ( bSlavesReady )
		{	//all slaves are in Safe-Operational
			//-->execute the init commands for the transition Pre-Operational to Safe-Operational
			stateValue	= ECAT_INITCMD_P_S;
			//next state: Safe-Operational
			stateNext	= EC_MASTER_STATE_SAFEOP;
		}
		break;

	case EC_MASTER_STATE_SAFEOP:
		switch ( pEcMaster->m_reqState )
		{
		case DEVICE_STATE_INIT:
			stateValue	= ECAT_INITCMD_S_I|ECAT_INITCMD_BEFORE;
			stateNext	= EC_MASTER_STATE_WAIT_SLAVE_S_I;
			break;
		case DEVICE_STATE_PREOP:
			stateValue	= ECAT_INITCMD_S_P|ECAT_INITCMD_BEFORE;
			stateNext	= EC_MASTER_STATE_WAIT_SLAVE_S_P;
			break;
		case DEVICE_STATE_SAFEOP:
			bMasterReady = 1;
			break;
		case DEVICE_STATE_OP:
			stateValue	= ECAT_INITCMD_S_O|ECAT_INITCMD_BEFORE;
			stateNext	= EC_MASTER_STATE_WAIT_SLAVE_S_O;
			break;
		}
		break;
	case EC_MASTER_STATE_WAIT_SLAVE_S_I:
		if ( bSlavesReady )
		{
			stateValue	= ECAT_INITCMD_S_I;
			stateNext	= EC_MASTER_STATE_INIT;
		}
		break;
	case EC_MASTER_STATE_WAIT_SLAVE_S_P:
		if ( bSlavesReady )
		{
			stateValue	= ECAT_INITCMD_S_P;
			stateNext	= EC_MASTER_STATE_PREOP;
		}
		break;
	case EC_MASTER_STATE_WAIT_SLAVE_S_O:
		if ( bSlavesReady )
		{
			stateValue	= ECAT_INITCMD_S_O;
			stateNext	= EC_MASTER_STATE_OP;
		}
		break;

	case EC_MASTER_STATE_OP:
		switch ( pEcMaster->m_reqState )
		{
		case DEVICE_STATE_INIT:
			stateValue	= ECAT_INITCMD_O_I|ECAT_INITCMD_BEFORE;
			stateNext	= EC_MASTER_STATE_WAIT_SLAVE_O_I;
			break;
		case DEVICE_STATE_PREOP:
			stateValue	= ECAT_INITCMD_O_P|ECAT_INITCMD_BEFORE;
			stateNext	= EC_MASTER_STATE_WAIT_SLAVE_O_P;
			break;
		case DEVICE_STATE_SAFEOP:
			stateValue	= ECAT_INITCMD_O_S|ECAT_INITCMD_BEFORE;
			stateNext	= EC_MASTER_STATE_WAIT_SLAVE_O_S;
			break;
		case DEVICE_STATE_OP:
			bMasterReady = 1;
			if ( pEcMaster->m_dcSyncMode == ECAT_DC_MODE_MASTER )
			{
				unsigned int i;
				switch (pEcMaster->m_eDcState)
				{
				case EC_DC_TIMING_INIT:
					//t.Trace(TraceInfo,"EC_DC_TIMING_INIT\n" );
					pEcMaster->m_nDcTimingCounter = 0;
					if ( pEcMaster->m_enableContinuousMeasuring )
						pEcMaster->m_eDcState = EC_DC_TIMING_IDLE;
					break;
				case EC_DC_TIMING_IDLE:
					//t.Trace(TraceInfo,"EC_DC_TIMING_IDLE\n" );
					pEcMaster->m_nDcTimingCounter++;
					if ( pEcMaster->m_nDcTimingCounter >= 10000 )
////					if ( pEcMaster->m_nDcTimingCounter >= 2000 )
					{
						pEcMaster->m_nDcTimingCounter		= 0;
						pEcMaster->m_eDcState = EC_DC_TIMING_READ_TIMING;
						for ( i=0; i < pEcMaster->m_nEcSlave; i++ )
							ExecDcTimingEcSlave(pEcMaster->m_ppEcSlave[i], EC_DC_TIMING_ONLINE_INIT);
					}					
					break;
				case EC_DC_TIMING_READ_TIMING:
					//t.Trace(TraceInfo,"EC_DC_TIMING_READ_TIMING\n" );
					// read timings of the slaves 
					EcatCmdFlushEcMaster(pEcMaster);
					if ( (pEcMaster->m_nDcTimingCounter % 10) == 0 )
					{
						EcatCmdReqEcMaster(pEcMaster, NULL, ECMI_DC_INIT, EC_CMD_TYPE_BWR, 0, ESC_REG_DC_SOF_LATCH_A, sizeof(ULONGLONG), NULL);
						EcatCmdFlushEcMaster(pEcMaster);
					}
					if ( (pEcMaster->m_nDcTimingCounter % 10) == 2 )
					{
						for ( i=0; i < pEcMaster->m_nEcSlave; i++ )
							ExecDcTimingEcSlave(pEcMaster->m_ppEcSlave[i], EC_DC_TIMING_READ_TIMING);
						EcatCmdFlushEcMaster(pEcMaster);
					}

					pEcMaster->m_nDcTimingCounter++;

					if ( pEcMaster->m_nDcTimingCounter >= ECAT_DC_READ_TIMING_CNT_OP )
						pEcMaster->m_eDcState = EC_DC_TIMING_CALC_TIMING;

					break;
				case EC_DC_TIMING_CALC_TIMING:
					//t.Trace(TraceInfo,"EC_DC_TIMING_CALC_TIMING\n" );
					CalcDcTimingsEcMaster(pEcMaster);
					pEcMaster->m_eDcState = EC_DC_TIMING_WRITE_CLOCK_DELAY;
					break;
				case EC_DC_TIMING_WRITE_CLOCK_DELAY:

					//t.Trace(TraceInfo,"++++++++ EC_DC_TIMING_WRITE_CLOCK_DELAY  %d\n", pEcMaster->m_nEcSlave );

					for ( i=0; i < pEcMaster->m_nEcSlave; i++ )
					{

						// t.Trace(TraceInfo,"!!!!! EC_DC_TIMING_WRITE_CLOCK_DELAY %s %d\n, pEcMaster->m_ppEcSlave[i]->m_szName,  pEcMaster->m_ppEcSlave[i]->m_bDcRefClock" );
						ExecDcTimingEcSlave(pEcMaster->m_ppEcSlave[i], EC_DC_TIMING_WRITE_CLOCK_DELAY);
					}

					pEcMaster->m_eDcState = EC_DC_TIMING_DONE;
					break;
				case EC_DC_TIMING_DONE:
					//t.Trace(TraceInfo,"EC_DC_TIMING_DONE\n" );
					pEcMaster->m_eDcState	= EC_DC_TIMING_INIT;
					break;
				}
			}
			break;
		
		}
		break;
	case EC_MASTER_STATE_WAIT_SLAVE_O_I:
		if ( bSlavesReady )
		{
			stateValue	= ECAT_INITCMD_O_I;
			stateNext	= EC_MASTER_STATE_INIT;
		}
		break;
	case EC_MASTER_STATE_WAIT_SLAVE_O_P:
		if ( bSlavesReady )
		{
			stateValue	= ECAT_INITCMD_O_P;
			stateNext	= EC_MASTER_STATE_PREOP;
		}
		break;
	case EC_MASTER_STATE_WAIT_SLAVE_O_S:
		if ( bSlavesReady )
		{
			stateValue	= ECAT_INITCMD_O_S;
			stateNext	= EC_MASTER_STATE_SAFEOP;
		}
		break;
	}
	
	if ( stateValue != EC_MASTER_STATE_NONE && pEcMaster->m_cInitCmds == INITCMD_INACTIVE )
	{  //stateValue specifies a transition in which init commands should be sent
		//and no init command is being processed at the momement.
		pEcMaster->m_cInitCmds = 0;
		StartInitCmdsEcMaster(pEcMaster, stateValue, stateNext);		
	}
	return (bMasterReady && bSlavesReady) ? 1 : 0;
}



//long max (long a, long b );


///\brief Send cyclic frames
void	SendCycFramesEcMaster(PEcMaster pEcMaster) 
{
	unsigned long i;
	PEcDevice    pEcDevice;

	pEcDevice =  (PEcDevice)pEcMaster->m_ipDev;

	//iteration of cyclic frames
	for ( i=0; i < pEcMaster->m_nEcCycInfo; i++ )
	{
		unsigned short state=0;
		//get current state of master
		state = GetStateMachineEcMaster(pEcMaster);

		if ( (state & pEcMaster->m_ppEcCycInfo[i]->state) == 0 )
		{	// frame is not queued in this cycle
			continue;	// jump to next frame
		}

		unsigned long  actSize=ETHERNET_88A4_FRAME_LEN + pEcMaster->m_ppEcCycInfo[i]->e88A4Frame.E88A4.Length;
		if ( actSize <= ETHERNET_MAX_FRAME_LEN )
		{	// frame should be send
			long vlanAdd = pEcMaster->m_ppEcCycInfo[i]->vlanInfo.VLanType != 0 ? ETYPE_VLAN_HEADER_LEN : 0;

			actSize += vlanAdd;
			unsigned char * pFrame = (unsigned char *)FrameAllocEcDevice(pEcDevice, actSize);
			if ( pFrame )
			{
				PETHERNET_88A4_MAX_HEADER pE88A = NULL;
				if ( vlanAdd == 0 )
				{
					memcpy(&pFrame[0], &pEcMaster->m_ppEcCycInfo[i]->e88A4Frame, ETHERNET_88A4_FRAME_LEN);
					pE88A = (PETHERNET_88A4_MAX_HEADER)&pFrame[ETHERNET_FRAME_LEN];
				}
				else
				{
					memcpy(&pFrame[0], &pEcMaster->m_ppEcCycInfo[i]->e88A4Frame, 2*ETHERNET_ADDRESS_LEN);
					memcpy(&pFrame[2*ETHERNET_ADDRESS_LEN], &pEcMaster->m_ppEcCycInfo[i]->vlanInfo, ETYPE_VLAN_HEADER_LEN);
					memcpy(&pFrame[2*ETHERNET_ADDRESS_LEN+ETYPE_VLAN_HEADER_LEN], &pEcMaster->m_ppEcCycInfo[i]->e88A4Frame.Ether.FrameType, 
						ETHERNET_FRAMETYPE_LEN+ETYPE_88A4_HEADER_LEN);
					pE88A = (PETHERNET_88A4_MAX_HEADER)&pFrame[ETHERNET_VLAN_FRAME_LEN];
				}
				
				//get pointer of First EtherCAT Command
				unsigned char * pPtr = (unsigned char *)&pE88A->FirstEcHead;
				PETYPE_EC_HEADER pLast = NULL;
				//iteration of commands
				for ( unsigned long j=0; j < pEcMaster->m_ppEcCycInfo[i]->cmdCnt; j++ )
				{
					PEcCmdInfo pInfo = EcCycCmdInfo(pEcMaster->m_ppEcCycInfo[i], j);

					/*if ( (state & pInfo->state) == 0 )
					{	// command is not queued in this cycle -> adjust size
						pE88A->E88A4.Length	-= pInfo->cmdSize;
						actSize					-= pInfo->cmdSize;
						continue;	// jump to next command
					}*/

					PETYPE_EC_HEADER pHead	= (PETYPE_EC_HEADER)pPtr;
					*pHead						= pInfo->cmdHeader;

					pHead->next = 1;

					pLast = pHead;

					pPtr += ETYPE_EC_HEADER_LEN;
					
                                        if ( (state & pInfo->state) == 0 )
						pHead->cmd = EC_CMD_TYPE_NOP;				
					else
						pHead->cmd = pInfo->cmdHeader.cmd;

					if ( pInfo->copyOutputs && state == DEVICE_STATE_OP)
						//copy data from output process image to the cyclic cmd
						memcpy(pPtr, pInfo->pCycOutputs, pHead->len);
					else
						memset(pPtr, 0, pHead->len);
					pPtr += pHead->len;
					*(unsigned short *)pPtr = pInfo->cntSend;
					pPtr += sizeof(unsigned short);
				}

				if ( pLast )
				{
					pLast->next = 0;

					FrameSendAndFreeEcDevice(pEcDevice, pFrame, actSize);
	
				}
				else
				{	// all commands are not queued in this cycle
					FrameFreeEcDevice(pEcDevice, pFrame);
				}
			}
		}
	}
	//this timer is checked in CEcMaster::CyclicTimeoutElapsed() to 
	//determine if SendCycFrames has been called in the last 200ms
	//otherwise MbSlave::MBoxReadFromSlave() is called
// tullio andicappato
 	Start(pEcMaster->m_tCyclicSend, 200);
//	Start(pEcMaster->m_tCyclicSend, 5);
	
	return ;
}


///////////////////////////////////////////////////////////////////////////////
///\brief Processes init commands that should be sent in this transition.
void	StartInitCmdsEcMaster(PEcMaster pEcMaster,unsigned short stateValue)
{
	EC_MASTER_STATE stateNext;
	switch ( stateValue )
	{
	case ECAT_INITCMD_I_P|ECAT_INITCMD_BEFORE:	
		stateNext = EC_MASTER_STATE_WAIT_SLAVE_I_P;	
		stateNext = EC_MASTER_STATE_DCLOCKS;	
		pEcMaster->m_eDcState = EC_DC_TIMING_INIT;	
		break;
	case ECAT_INITCMD_I_P:	
	case ECAT_INITCMD_S_P:	
	case ECAT_INITCMD_O_P:	
		stateNext = EC_MASTER_STATE_PREOP;				
		break;
	case ECAT_INITCMD_P_I|ECAT_INITCMD_BEFORE:	
		stateNext = EC_MASTER_STATE_WAIT_SLAVE_P_I;	
		break;
	case ECAT_INITCMD_P_S|ECAT_INITCMD_BEFORE:	
		stateNext = EC_MASTER_STATE_WAIT_SLAVE_P_S;
		break;
	case ECAT_INITCMD_P_S:	
	case ECAT_INITCMD_O_S:	
		stateNext = EC_MASTER_STATE_SAFEOP;				
		break;
	case ECAT_INITCMD_S_I|ECAT_INITCMD_BEFORE:	
		stateNext = EC_MASTER_STATE_WAIT_SLAVE_S_I;
		break;
	case ECAT_INITCMD_S_P|ECAT_INITCMD_BEFORE:	
		stateNext = EC_MASTER_STATE_WAIT_SLAVE_S_P;
		break;
	case ECAT_INITCMD_S_O|ECAT_INITCMD_BEFORE:	
		stateNext = EC_MASTER_STATE_WAIT_SLAVE_S_O;
		break;
	case ECAT_INITCMD_S_O:	
		stateNext = EC_MASTER_STATE_OP;					
		break;
	case ECAT_INITCMD_O_I|ECAT_INITCMD_BEFORE:	
		stateNext = EC_MASTER_STATE_WAIT_SLAVE_O_I;
		break;
	case ECAT_INITCMD_O_P|ECAT_INITCMD_BEFORE:	
		stateNext = EC_MASTER_STATE_WAIT_SLAVE_O_P;	
		break;
	case ECAT_INITCMD_O_S|ECAT_INITCMD_BEFORE:	
		stateNext = EC_MASTER_STATE_WAIT_SLAVE_O_S;	
		break;
	}
	StartInitCmdsEcMaster( pEcMaster, stateValue, stateNext);
}

///////////////////////////////////////////////////////////////////////////////
///\brief Processes init commands that should be sent in this transition.
void	StartInitCmdsEcMaster(PEcMaster pEcMaster, unsigned short stateValue, EC_MASTER_STATE stateNext)
{	
	//check if an init command is defined for the transition specified in stateValue
	while ( pEcMaster->m_cInitCmds < pEcMaster->m_nInitCmds )
	{
		//check if Init command exists for this transition and if ECAT_INITCMD_BEFHORE is set
		//in m_ppInitCmds[m_cInitCmds]->transition as well as in stateValue
		if ( (pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds]->transition & (stateValue|ECAT_INITCMD_BEFORE)) == stateValue ) 
			//init command found --> process the first init command for this transition
			break;
		pEcMaster->m_cInitCmds++;
	}
	if ( pEcMaster->m_cInitCmds < pEcMaster->m_nInitCmds )
	{	//init command found
		PEcInitCmdDesc p	= pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds];
		//Set timeout for init commands
		Start(pEcMaster->m_tInitCmds, p->timeout);	
		//send first init command defined for this transition
		EcatCmdReqEcMaster( pEcMaster, NULL, stateValue, p->ecHead.cmd, p->ecHead.adp, p->ecHead.ado, 
			p->ecHead.len, EcInitCmdDescData(p));
		
		//the other init commands defined for this transition will be sent in EcMaster::EcatCmdRes after
		//receiving the response to this frame			
	}
	else
	{
		pEcMaster->m_cInitCmds = INITCMD_INACTIVE;
		unsigned short slaveReq=0;

		//determine which state the master is trying to change to 
		//and request this stat from all CEcSlave objects.
		switch ( stateNext )
		{
		case EC_MASTER_STATE_WAIT_SLAVE_P_I:	
		case EC_MASTER_STATE_WAIT_SLAVE_S_I:	
		case EC_MASTER_STATE_WAIT_SLAVE_O_I:	slaveReq = DEVICE_STATE_INIT;	break;
		case EC_MASTER_STATE_WAIT_SLAVE_I_P:	
		case EC_MASTER_STATE_WAIT_SLAVE_S_P:	
		case EC_MASTER_STATE_WAIT_SLAVE_O_P:	slaveReq = DEVICE_STATE_PREOP;	break;
		case EC_MASTER_STATE_WAIT_SLAVE_P_S:	
		case EC_MASTER_STATE_WAIT_SLAVE_O_S:	slaveReq = DEVICE_STATE_SAFEOP;	break;
		case EC_MASTER_STATE_WAIT_SLAVE_S_O:	slaveReq = DEVICE_STATE_OP;	break;
		}
		if ( slaveReq )
		{
			slaveReq = slaveReq;
			for ( unsigned int i=0; i < pEcMaster->m_nEcSlave; i++ )
				RequestStateEcSlave(pEcMaster->m_ppEcSlave[i], slaveReq);
		}
		//change the current state of the master 
		pEcMaster->m_currState = stateNext;
	}
}
	




 
void CheckAsyncFrameEcMaster(PEcMaster pEcMaster)
{
	PEcMbSlave   pMbx;
	PEcDevice    pEcDevice;

	pEcDevice =  (PEcDevice)pEcMaster->m_ipDev;

	while ( pEcMaster->m_listReadSlaveMbx.Remove(pMbx) )
		MBoxReadFromSlaveEcMbSlave(pMbx);

	EcAcycFrameInfo info;
	while ( pEcMaster->m_listAcycFrame.Remove(info) )
	{
		unsigned short*	pFrameType	= FRAMETYPE_PTR(info.pFrame);
		PETHERNET_88A4_MAX_HEADER pE88A4 = (PETHERNET_88A4_MAX_HEADER)ENDOF(pFrameType);
		PETYPE_EC_HEADER pHead		= &pE88A4->FirstEcHead;
		
		PECAT_SLAVEFRAME_INFO pInfo = NULL;			
		if ( !IsListEmpty(&pEcMaster->m_listPend) )
			pInfo = (PECAT_SLAVEFRAME_INFO)RemoveHeadListAB(&pEcMaster->m_listPend);			
		if ( pInfo )
		{			
			while ( pInfo->idx != pHead->idx )
			{  //no response to pending pInfo received -> check next pending frame
				for ( unsigned int i=0; i < pInfo->nInfo; i++ )
				{
					if ( pInfo->cmdInfo[i].pSlave )
						//EtherCAT command originated from CEcSlave object -> inform CEcSlave object of error
						EcatCmdResEcSlave((PEcSlave) pInfo->cmdInfo[i].pSlave, ECERR_DEVICE_NOTREADY, 
							pInfo->cmdInfo[i].invokeId, NULL,0);
				}
				pInfo->frame.E88A4.Length	= 0;
				pInfo->pLastHead				= NULL;
				pInfo->nInfo					= 0;
				
				InsertTailListAB(&pEcMaster->m_listFree, (PLIST_ENTRY_AB)pInfo);
				pInfo								= NULL;
				if ( !IsListEmpty(&pEcMaster->m_listPend) )
					//set pInfo to next pending frame
					pInfo = (PECAT_SLAVEFRAME_INFO)RemoveHeadListAB(&pEcMaster->m_listPend);

				if ( pInfo == NULL )						
					//no pending frame left
					break;				
			}			
			if ( pInfo != NULL )						
			{	
				//iterate through all sub commands contained in the frame
				for ( unsigned int i=0; i < pInfo->nInfo; i++ )
				{
					if ( pInfo->cmdInfo[i].pSlave )
					{//EtherCAT command originated from CEcSlave object 
						if ( pHead )
						{
							//forward response to CEcSlave object
							EcatCmdResEcSlave((PEcSlave)pInfo->cmdInfo[i].pSlave, ECERR_NOERR, pInfo->cmdInfo[i].invokeId, pHead,0);							
							if ( pHead->next )
								//get next sub command
								pHead = NEXT_EcHeader(pHead);
							else
								pHead = NULL;
						}
						else
							//next sub command dose not exists -> report error to CEcSlave object
							EcatCmdResEcSlave((PEcSlave)pInfo->cmdInfo[i].pSlave, ECERR_DEVICE_ERROR, pInfo->cmdInfo[i].invokeId, NULL,0);
					}
					else
					{
						if ( pHead )
						{
							EcatCmdResEcMaster(pEcMaster, ECERR_NOERR, pInfo->cmdInfo[i].invokeId, pHead);

							if ( pHead->next )
								//get next sub command
								pHead = NEXT_EcHeader(pHead);
							else
								pHead = NULL;
						}
					}
				}
				pInfo->frame.E88A4.Length	= 0;
				pInfo->pLastHead				= NULL;
				pInfo->nInfo					= 0;
				
				//make pInfo available again for following EtherCAT command requests.
				InsertTailListAB(&pEcMaster->m_listFree, (PLIST_ENTRY_AB)pInfo);				
				pInfo								= NULL;
			}
			pEcDevice->m_pEcNpfDevice->ReturnFrame( info.pFrame);
		}		
	}
}

 
///////////////////////////////////////////////////////////////////////////////
///\brief Is called when a new message in a slave mailbox has been indicated.
int CheckMBoxCmdEcMaster(PEcMaster pEcMaster,PETYPE_EC_HEADER pHead)
{
	//find responsible mailbox slave 
	if ( pHead->laddr == pEcMaster->m_logAddressMBoxStates && pHead->len == (pEcMaster->m_sizeAddressMBoxStates+7)/8 )
	{
		unsigned char * pData = (unsigned char*)ENDOF(pHead);
		for ( int i=0; i < pHead->len; i+=sizeof(unsigned char) )
		{
			unsigned char data = pData[i];
			if ( data != 0 )
			{
				for ( int b=0; b < 8; b++ )
				{
					if ( (data & (1 << b)) != 0 )
					{//new data available in mailbox -> determine responsible CEcMbSlave::Object
						int idx = i*8 + b;
						if ( pEcMaster->m_ppEcPortMBoxState[idx] )
							//read mailbox from slave
							pEcMaster->m_listReadSlaveMbx.Add(pEcMaster->m_ppEcPortMBoxState[idx]);
					}
				}
			}
		}
	}
	return 1;
}


///////////////////////////////////////////////////////////////////////////////
///\brief Checks received frame.
int	CheckFrameEcMaster(PEcMaster pEcMaster, PETHERNET_FRAME pFrame, unsigned long nFrame, void * pProtocolBegin, int &bPending)
{
	PETHERNET_88A4_MAX_HEADER pE88A4 = (PETHERNET_88A4_MAX_HEADER)pProtocolBegin;
	bPending = 0;
	if ( 
			memcmp( pFrame->Destination.b, pEcMaster->m_macDest.b, sizeof(ETHERNET_ADDRESS) ) == 0  
		 ||	memcmp( pFrame->Source.b,      pEcMaster->m_macDest.b, sizeof(ETHERNET_ADDRESS) ) == 0  
		 
	  )
	{	// mac not switched                   mac switched
		HRESULT	hr						= 0;
		unsigned long e88A4Len					= pE88A4->E88A4.Length;
		PETYPE_EC_HEADER pHead	  		= &pE88A4->FirstEcHead;
		
		
		if ( pHead->idx >= EC_HEAD_IDX_SLAVECMD && pHead->idx < EC_HEAD_IDX_SLAVECMD + MAX_SLAVEFRM )
		{	
			// acyclic frame
			EcAcycFrameInfo info;
			info.pFrame			= pFrame;
			info.nFrame			= nFrame;

			if ( pEcMaster->m_listAcycFrame.Add(info) )
			{
				bPending = 1;
				return 1; 
			}
			else
			{
				return 0;
			}
		}
		/*else
		{	// cyclic command			
				while ( pHead )
			{	// look for each EtherCAT sub telegram
				if ( e88A4Len < ETYPE_EC_CMD_LEN(pHead) )
					break;
				PEcCmdInfo pInfo;
				if ( pEcMaster->m_mapEcCmdInfo.Lookup(EcCmdHash(pHead), pInfo) )
				{	// EtherCAT sub telegram found
					hr = 1;
					if ( pInfo->cyclic )
					{						
						if ( pInfo->mboxState )
							//command is responsible for reading out the state of the mailbox -> check if the mailbox of a slave has to be read out
							CheckMBoxCmdEcMaster(pEcMaster, pHead );

						if ( pInfo->copyInputs )
						{//command is responsible for reading out the inputs of one or more slaves -> copy data to process image
							void * pino = ENDOF(pHead);
							memcpy(pInfo->pCycInputs, pino, pHead->len);	
							//memcpy(pInfo->pCycInputs, ENDOF(pHead), pHead->len);	
						}	
						
/////////////////////////////////////////////////////////////////////////////////////////
// Gestione DC - acquisizione Time

						//one can overwrite this method in inherited class to do additional
						//checking of commands
						//e.g.
						//CEcMyMaster::CheckCyclicCmd(PEcCmdInfo pCmdInfo, PETYPE_EC_HEADER pHead, unsigned short wcCnt)
						//{	
 						if ( pHead->cmd == EC_CMD_TYPE_ARMW && pHead->ado == ESC_REG_DC_SYSTIME  )						
						{
							unsigned int iCosa;

							void * pino = ENDOF(pHead);
				//			memcpy(&EcatSysTime, pino, pHead->len);	

							memcpy(&EcatSysTime, pino, 4);	
							memcpy(&iCosa, pino, 4);	
							if ( iCosa < iPrecedente )
							{
								EcatSysTime += 0x100000000;
							}						
							iPrecedente = iCosa;
							LenEcatSysTime = pHead->len;

							SysTimeRead = 1;			// marco - setta il flag per il calcolo PI per tenere agganciato
						}						
						// l'Ethercat System Time con il timer Realtek
						//}	
					}
				}

				e88A4Len -= ETYPE_EC_CMD_LEN(pHead);

				if ( pHead->next )
					pHead = NEXT_EcHeader(pHead);
				else
					pHead = NULL;
			}
		}*/
      		else if ( pHead->idx>=EC_HEAD_IDX_CYCLIC_START && pHead->idx<EC_HEAD_IDX_CYCLIC_START+pEcMaster->m_nEcCycInfo )
      		{	// cyclic command			
      			hr = 1;
      			PEcCycInfo pCyc = pEcMaster->m_ppEcCycInfo[pHead->idx-EC_HEAD_IDX_CYCLIC_START];
      			if ( pE88A4->E88A4.Length == pCyc->e88A4Frame.E88A4.Length )
      			{
      				for ( int i=0; i<pCyc->cmdCnt && pHead; i++ )
      				{
      					PEcCmdInfo pCmd = EcCycCmdInfo(pCyc, i);
      					if ( pCmd->cmdHeader.len == pHead->len )
      					{
      						if ( pCmd->cyclic )
      						{						
      							if ( pCmd->mboxState )
      								//command is responsible for reading out the state of the mailbox -> check if the mailbox of a slave has to be read out
      								CheckMBoxCmdEcMaster(pEcMaster, pHead );
      
      							if ( pCmd->copyInputs )
      							{//command is responsible for reading out the inputs of one or more slaves -> copy data to process image
      								void * pino = ENDOF(pHead);
      								memcpy(pCmd->pCycInputs, pino, pHead->len);	
      								//memcpy(pInfo->pCycInputs, ENDOF(pHead), pHead->len);
      							}	
      							
      	/////////////////////////////////////////////////////////////////////////////////////////
      	// Gestione DC - acquisizione Time
      
      							//one can overwrite this method in inherited class to do additional
      							//checking of commands
      							//e.g.
      							//CEcMyMaster::CheckCyclicCmd(PEcCmdInfo pCmdInfo, PETYPE_EC_HEADER pHead, USHORT wcCnt)
      							//{	
       							if ( pHead->cmd == EC_CMD_TYPE_ARMW && pHead->ado == ESC_REG_DC_SYSTIME  )						
      							{
      								unsigned int iCosa;
      
      								void * pino = ENDOF(pHead);
      					//			memcpy(&EcatSysTime, pino, pHead->len);	
      
      								memcpy(&EcatSysTime, pino, 4);	
      								memcpy(&iCosa, pino, 4);	
      								if ( iCosa < iPrecedente )
      								{
      									EcatSysTime += 0x100000000;
      								}						
      								iPrecedente = iCosa;
      								LenEcatSysTime = pHead->len;
      
      								SysTimeRead = 1;			// marco - setta il flag per il calcolo PI per tenere agganciato
      							}						
      							// l'Ethercat System Time con il timer Realtek
      							//}	
      						}
      					}
      					else
      						break;
      
      					if ( pHead->next )
      						pHead = NEXT_EcHeader(pHead);
      					else
      						pHead = NULL;
      				}
      			}			
      		}
      		else
      		{//Unknown command
      			//EC_TRACE(_T("CEcMaster::CheckFrame; ******* Unknown Commad idx = 0x%x!\n"), pHead->idx);		
      		}	
		return hr;
	}
	else
		return 0;
}

 
///////////////////////////////////////////////////////////////////////////////
///\brief Processes the response of an EtherCAT command.
void	EcatCmdResEcMaster(PEcMaster pEcMaster, unsigned long result, unsigned long invokeId, PETYPE_EC_HEADER pHead)
{
	unsigned int msg = EC_LOG_MSGTYPE_LOG|EC_LOG_MSGTYPE_ERROR;
	switch ( invokeId )
	{
	case ECMI_DC_INIT:	
		break;
	case ECAT_INITCMD_I_P:
	case ECAT_INITCMD_P_S:
	case ECAT_INITCMD_P_I:
	case ECAT_INITCMD_S_P:
	case ECAT_INITCMD_S_O:
	case ECAT_INITCMD_S_I:
	case ECAT_INITCMD_O_S:
	case ECAT_INITCMD_O_P:
	case ECAT_INITCMD_O_I:
	case ECAT_INITCMD_I_P|ECAT_INITCMD_BEFORE:
	case ECAT_INITCMD_P_S|ECAT_INITCMD_BEFORE:
	case ECAT_INITCMD_P_I|ECAT_INITCMD_BEFORE:
	case ECAT_INITCMD_S_P|ECAT_INITCMD_BEFORE:
	case ECAT_INITCMD_S_O|ECAT_INITCMD_BEFORE:
	case ECAT_INITCMD_S_I|ECAT_INITCMD_BEFORE:
	case ECAT_INITCMD_O_S|ECAT_INITCMD_BEFORE:
	case ECAT_INITCMD_O_P|ECAT_INITCMD_BEFORE:
	case ECAT_INITCMD_O_I|ECAT_INITCMD_BEFORE:
		if ( pEcMaster->m_cInitCmds != INITCMD_INACTIVE && pEcMaster->m_cInitCmds < pEcMaster->m_nInitCmds )
		{
			if ( result == ECERR_NOERR )
			{
				//check working counter if necessary ( cnt != 0xFFFF (ECAT_WCOUNT_DONT_CHECK) )
				if ( pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds]->cnt != 0xFFFF && ETYPE_EC_CMD_COUNT(pHead) != pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds]->cnt )
				{	//valid working counter
					if ( pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds]->retries > 0 )
					{	// retry
						PEcInitCmdDesc p = pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds];
						//send EtherCAT command again
						EcatCmdReqEcMaster(pEcMaster, NULL, invokeId, p->ecHead.cmd, p->ecHead.adp, p->ecHead.ado, 
							p->ecHead.len, EcInitCmdDescData(p));
						pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds]->retries--;
					}
					else
					{						
						pEcMaster->m_reqState	= pEcMaster->m_currState;
						pEcMaster->m_cInitCmds	= INITCMD_INACTIVE;
					}
					break;
				}
				
				int bNext = 1;
				if ( pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds]->validate && pHead->len == pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds]->ecHead.len )
				{	// initcmd with validation
					unsigned char* pData = (unsigned char*)ENDOF(pHead);
					unsigned long nmData[2];
					if ( pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds]->validateMask )
					{
						if ( pHead->len <= sizeof(nmData) )
							memcpy(nmData, pData, pHead->len);
						unsigned char* pMask = (unsigned char*)EcInitCmdDescVMData(pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds]);
						for ( int i=0; i < pHead->len; i++ )
							pData[i] &= pMask[i];
					}
					if ( memcmp(pData, EcInitCmdDescVData(pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds]), pHead->len) != 0 )
					{	// validate failed -> retry

                                                iChi = 2;        
						if ( IsElapsed( pEcMaster->m_tInitCmds) )
						{						
							pEcMaster->m_reqState	= pEcMaster->m_currState;
							pEcMaster->m_cInitCmds	= INITCMD_INACTIVE;
						}
						else
						{
							PEcInitCmdDesc p = pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds];
							EcatCmdReqEcMaster(pEcMaster, NULL, invokeId, p->ecHead.cmd, p->ecHead.adp, p->ecHead.ado, 
								p->ecHead.len, EcInitCmdDescData(p));						
						}
						bNext = 0;
					}
				}
				if ( bNext )
				{
					pEcMaster->m_cInitCmds++;
					StartInitCmdsEcMaster( pEcMaster,(unsigned short)invokeId);
				}
			}
			else
			{
				if ( pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds]->retries > 0 )
				{	// retry
					PEcInitCmdDesc p = pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds];
					EcatCmdReqEcMaster(pEcMaster, NULL, invokeId, p->ecHead.cmd, p->ecHead.adp, p->ecHead.ado, 
						p->ecHead.len, EcInitCmdDescData(p));
					pEcMaster->m_ppInitCmds[pEcMaster->m_cInitCmds]->retries--;
				}
				else
				{					
					pEcMaster->m_reqState	= pEcMaster->m_currState;
					pEcMaster->m_cInitCmds	= INITCMD_INACTIVE;
				}
			}
		}
		break;
	}
}

 
//////////////////////////////////////////////////////////////////
///\brief Sends a mailbox command to the specified slave.
long	OnMailboxReceiveEcMaster( PEcMaster pEcMaster,PEcMailboxCmd pCmd )
{
	long nRes = ECERR_DEVICE_INVALIDADDR;
	if( pCmd->addr.port != pEcMaster->m_mailbox.m_addr.port) 
	{//mailbox command directed at slave
		if( pCmd->addr.port < pEcMaster->m_nEcSlave )			
		{
			nRes = MailboxReceiveFromMasterEcMbSlave( pEcMaster->m_ppEcSlave[pCmd->addr.port], pCmd);						
		}
	}
	//else 
	//->mailbox command directed at master	
	
	if( nRes != ECERR_NOERR )
		delete[] pCmd; 
		

	return nRes;
}



///////////////////////////////////////////////////////////////////////////////
///\brief Register or unregister mailbox polling for the specified EtherCAT slave.
int	RegisterMBoxPollingEcMaster( PEcMaster pEcMaster, EcMbSlave * pSlave, unsigned short slaveAddressMBoxState, int bUnregister)
{
	if ( slaveAddressMBoxState < pEcMaster->m_sizeAddressMBoxStates*8 )
	{
		if ( bUnregister )
		{
			if ( pEcMaster->m_ppEcPortMBoxState[slaveAddressMBoxState] != NULL )
			{
				pEcMaster->m_ppEcPortMBoxState[slaveAddressMBoxState] = NULL;
				return 1;
			}
		}
		else
		{
			if ( pEcMaster->m_ppEcPortMBoxState[slaveAddressMBoxState] == NULL )
			{
				pEcMaster->m_ppEcPortMBoxState[slaveAddressMBoxState] = pSlave;
				return 1;
			}
		}
	}
	return 0;
}
 
///////////////////////////////////////////////////////////////////////////////
///\brief Is called by CycleMBoxPolling() to check if the timeout m_tCyclicSend for the cylic commands has elapsed.
int CyclicTimeoutElapsedEcMaster(PEcMaster pEcMaster )
{
        iChi = 3;   
	return GetStateMachineEcMaster(pEcMaster) != DEVICE_STATE_INIT && IsElapsed(pEcMaster->m_tCyclicSend);
}



///////////////////////////////////////////////////////////////////////////////
///\brief Queues pending EtherCAT commands stored in m_cmdInfos
long	EcatCmdFlushEcMaster(PEcMaster pEcMaster) 
{
	PEcDevice pEcDevice;

	pEcDevice = (PEcDevice)pEcMaster->m_ipDev;

	if ( pEcMaster->m_pPend != NULL )
	{	
		unsigned long nFrame = SIZEOF_88A4_FRAME(&pEcMaster->m_pPend->frame);
		PETHERNET_88A4_FRAME pFrame = (PETHERNET_88A4_FRAME) FrameAllocEcDevice( pEcDevice, nFrame);
		if ( pFrame )
		{
			if ( !pEcMaster->m_pPend->timeout)
			{																
				pEcMaster->m_pPend->timeout = new TimeoutHelper; 
			}	
			Start( pEcMaster->m_pPend->timeout, TIMEOUT_RELOAD);


			pEcMaster->m_pPend->retry			= 2;
			memcpy(pFrame, &pEcMaster->m_pPend->frame, nFrame);
			if ( QueueE88A4CmdEcDevice(pEcDevice, pFrame) )
			{
				InsertTailListAB(&pEcMaster->m_listPend, (PLIST_ENTRY_AB)pEcMaster->m_pPend);
				if ( pEcMaster->m_pPend->timeout )
				{
					delete pEcMaster->m_pPend->timeout;
					pEcMaster->m_pPend->timeout = NULL;
				}
				pEcMaster->m_pPend = NULL;				
			}
			else
			{
				if ( pEcMaster->m_pPend->timeout )
				{
					delete pEcMaster->m_pPend->timeout;
					pEcMaster->m_pPend->timeout = NULL;
				}
				FrameFreeEcDevice( pEcDevice, (PETHERNET_88A4_MAX_FRAME)pFrame);
				return ECERR_DEVICE_BUSY;
			}
		}
		else
		{
			return ECERR_DEVICE_NOMEMORY;
		}
	}
	return ECERR_NOERR;
	
}

///////////////////////////////////////////////////////////////////////////////
///\brief Process EtherCAT command request. 
long	EcatCmdReqEcMaster(PEcMaster pEcMaster, EcSlave* pSlave, unsigned long invokeId, unsigned char cmd,
						    unsigned short adp, unsigned short ado, unsigned short len, void * pData, 
							unsigned short cnt, PETHERCAT_MBOX_HEADER pMbox, unsigned int pEthernet,
							PETHERCAT_CANOPEN_HEADER pCANopen,
							PETHERCAT_SDO_HEADER pSDO, 
							PETHERCAT_FOE_HEADER pFoE, 
							PETHERCAT_SOE_HEADER pSoE
						  )

{
	if ( len + ETYPE_EC_OVERHEAD > sizeof(ETHERNET_88A4_MAX_FRAME)-ETHERNET_FRAME_LEN-ETYPE_88A4_IO_HEADER_LEN )
		return ECERR_DEVICE_INVALIDSIZE;
	
	//EcatCmdReq can be either be called by CEcMaster::OnTimer or CEcDevice::GetIoState -> CEcDevice::CheckFrame -> CEcMaster::CheckFrame
	//->CEcMaster::EcatCmdRes or CEcSlave::EcatCmdRes	

	if ( pEcMaster->m_pPend != NULL )
	{//pending frame exists, that has not been sent jet
		if ( (pEcMaster->m_pPend->nSpace < len + ETYPE_EC_OVERHEAD || pEcMaster->m_pPend->nInfo == MAX_SLAVECMD) )
		{	//sub telegram does not fit into pending frame -> send pending frame
			long result = EcatCmdFlushEcMaster( pEcMaster);
			if ( result != ECERR_NOERR )
				return result;
		}
	}

	if ( pEcMaster->m_pPend == NULL )
	{//initialize new pending frame. 		
		if ( !IsListEmpty(&pEcMaster->m_listFree) )
			pEcMaster->m_pPend = (PECAT_SLAVEFRAME_INFO)RemoveHeadListAB(&pEcMaster->m_listFree);
		

		if ( pEcMaster->m_pPend == NULL )
			return ECERR_DEVICE_NOMOREHDLS;

		pEcMaster->m_pPend->frame.Ether.Source	= pEcMaster->m_macSrc;
		pEcMaster->m_pPend->frame.E88A4.Length	= 0;
		pEcMaster->m_pPend->nInfo						= 0;
		//calculate number of Bytes this frame has left for the EtherCAT commands
		//	m_maxAsynFrameSize = ETHERNET_MAX_FRAME_LEN = 1514 
		//	nSpace = 1514 - 14  - 2 = 1498 Bytes
		pEcMaster->m_pPend->nSpace					= pEcMaster->m_maxAsyncFrameSize - ETHERNET_FRAME_LEN - ETYPE_88A4_HEADER_LEN;		
		pEcMaster->m_pPend->pLastHead				= NULL;
	}

	if ( pEcMaster->m_pPend->pLastHead )
	{
		pEcMaster->m_pPend->pLastHead->next	= 1;
		pEcMaster->m_pPend->pLastHead			= NEXT_EcHeader(pEcMaster->m_pPend->pLastHead);
		pEcMaster->m_pPend->pLastHead->idx		= 0;
	}
	else
	{	//new pending frame -> set pLastHead to to the first EtherCAT-Header(directly after 88A4-Header)
		pEcMaster->m_pPend->pLastHead			= &pEcMaster->m_pPend->frame.FirstEcHead;
		//each entry ECAT_SLAVEFRAME_INFO in m_listFree has a different idx number(see constructor)
		pEcMaster->m_pPend->pLastHead->idx		= pEcMaster->m_pPend->idx;
	}

	pEcMaster->m_pPend->pLastHead->cmd		= cmd;
	pEcMaster->m_pPend->pLastHead->adp		= adp;
	pEcMaster->m_pPend->pLastHead->ado		= ado;
	pEcMaster->m_pPend->pLastHead->irq		= 0;
	pEcMaster->m_pPend->pLastHead->length	= len; // res and next = 0;
	
	if ( len > 0 )
	{
		unsigned char fillByte = 0x00;

		if ( pMbox )
		{//mailbox command
			unsigned char * pByte		= (unsigned char *)ENDOF(pEcMaster->m_pPend->pLastHead);
			unsigned int bytesLeft = len;
			unsigned int bytesData = pMbox->Length;

			//copy Mailbox header to the end of the EtherCAT command
			memcpy(pByte, pMbox, ETHERCAT_MBOX_HEADER_LEN);
			pByte			 += ETHERCAT_MBOX_HEADER_LEN;
			bytesLeft	     -= ETHERCAT_MBOX_HEADER_LEN;
			if ( pCANopen )
			{
				//copy CANopen over EtherCAT header to the end of the EtherCAT command
				memcpy(pByte, pCANopen, ETHERCAT_CANOPEN_HEADER_LEN);
				pByte			+= ETHERCAT_CANOPEN_HEADER_LEN;
				bytesLeft	    -= ETHERCAT_CANOPEN_HEADER_LEN;
				bytesData	    -= ETHERCAT_CANOPEN_HEADER_LEN;
				if ( pSDO )
				{
					//copy Sdo header to the end of the EtherCAT command
					memcpy(pByte, pSDO, ETHERCAT_SDO_HEADER_LEN);
					pByte			+= ETHERCAT_SDO_HEADER_LEN;
					bytesLeft	    -= ETHERCAT_SDO_HEADER_LEN;
					bytesData	    -= ETHERCAT_SDO_HEADER_LEN;
				}
			}
			else if ( pFoE )
			{
				memcpy(pByte, pFoE, ETHERCAT_FOE_HEADER_LEN);
				pByte			+= ETHERCAT_FOE_HEADER_LEN;
				bytesLeft	    -= ETHERCAT_FOE_HEADER_LEN;
				bytesData	    -= ETHERCAT_FOE_HEADER_LEN;
			}
			else if ( pSoE )
			{
				memcpy(pByte, pSoE, ETHERCAT_SOE_HEADER_LEN);
				pByte			+= ETHERCAT_SOE_HEADER_LEN;
				bytesLeft	    -= ETHERCAT_SOE_HEADER_LEN;
				bytesData	    -= ETHERCAT_SOE_HEADER_LEN;
			}
			if ( pData && bytesData > 0 )
			{	//copy data to the end of the EtherCAT command
				memcpy(pByte, pData, bytesData);
				pByte			+= bytesData;
				bytesLeft	    -= bytesData;
			}
			if ( bytesLeft > 0 )
				memset(pByte, fillByte, bytesLeft);
		}
		else
		{
			//copy data to the end of the EtherCAT command
			if ( pData )
				memcpy(ENDOF(pEcMaster->m_pPend->pLastHead), pData, len);
			else
				memset(ENDOF(pEcMaster->m_pPend->pLastHead), fillByte, len);
		}
	}
		
	//set working counter of command (normally cnt = 0 )
	ETYPE_EC_CMD_COUNT(pEcMaster->m_pPend->pLastHead)		= cnt;
	pEcMaster->m_pPend->cmdInfo[pEcMaster->m_pPend->nInfo].pSlave	= pSlave;
	pEcMaster->m_pPend->cmdInfo[pEcMaster->m_pPend->nInfo].invokeId	= invokeId;
	pEcMaster->m_pPend->nInfo++;
	pEcMaster->m_pPend->nSpace						    	-= len + ETYPE_EC_OVERHEAD;
	pEcMaster->m_pPend->frame.E88A4.Length					+= len + ETYPE_EC_OVERHEAD;

	return ECERR_NOERR;
}

///////////////////////////////////////////////////////////////////////////////
///\brief Process EtherCAT command request. 
long	EcatCmdReqEcMaster(PEcMaster pEcMaster, EcSlave* pSlave, unsigned long invokeId, unsigned char cmd, unsigned short adp, unsigned short ado, unsigned short data)
{
	return EcatCmdReqEcMaster(pEcMaster, pSlave, invokeId, cmd, adp, ado, sizeof(data), &data, 0);
}
 


void  MailboxResponseEcMaster( PEcMaster pEcMaster, PEcMailboxCmd pCmd)
{ 
	PEcMailbox	pMailbox;

	if( pCmd )	
	{ 
		pMailbox = &pEcMaster->m_mailbox;
		if( pMailbox )
		{
			OnMailboxResponseEcSyncClient( (PEcSyncClient)pMailbox->m_pClient, pCmd, pMailbox );		
		}			
		MailboxFree(pCmd);
	}	
}	



