#include <SPI.h>         // needed for Ethernet library communication with the W5100 (Arduino ver>0018)

#include "AxesBrainEcInterface.h"
#include "Strutture.h"
#include "AxesBrainEcNpfDevice.h"
#include "AxesBrainEcDevice.h"
#include "AxesBrainEcNpfDevice.h"

extern short                     iChi;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////       MailBox Slave             ///////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
unsigned short minS (unsigned short  a, unsigned short  b );
unsigned short maxS (unsigned short  a, unsigned short  b );


///////////////////////////////////////////////////////////////////////////////

void CancellaEcMbSlave(PEcMbSlave pEcMbSlave) 
{
	PEcMailboxCmd pCmd;

	if ( pEcMbSlave->m_stateMbxPolling )
		RegisterMBoxPollingEcMaster( (PEcMaster)pEcMbSlave->m_pEcSlave->m_pMaster , pEcMbSlave , pEcMbSlave->m_slaveAddressMBoxState, 1);

 	safe_delete_a(pEcMbSlave->m_ppMbxCmds);
	safe_delete_a(pEcMbSlave->m_pMbxCmds);

	if ( pEcMbSlave->m_pFoE )
	{
		ClearFoESafeEcMbSlave( pEcMbSlave);
		while ( pEcMbSlave->m_pFoE->pPend->Remove(pCmd) )
			//m_pMaster->GetMailbox()->MailboxFree(pCmd);
			delete [] pCmd;
		safe_delete(pEcMbSlave->m_pFoE->pPend);				
		safe_delete(pEcMbSlave->m_pFoE);
	}
	if ( pEcMbSlave->m_pCoE )
	{	
		ClearCoESafeEcMbSlave( pEcMbSlave);
		while ( pEcMbSlave->m_pCoE->pPend->Remove(pCmd) )
			//m_pMaster->GetMailbox()->MailboxFree(pCmd);
			delete [] pCmd;
		safe_delete(pEcMbSlave->m_pCoE->pPend);		
		safe_delete(pEcMbSlave->m_pCoE);
	}	

	if ( pEcMbSlave->m_pSoE )
	{
		ECMBSLAVE_SOE_CMD_INFO info;
		ClearSoESafeEcMbSlave( pEcMbSlave);
		while ( pEcMbSlave->m_pSoE->pPend->Remove(pCmd) )
			//m_pMaster->GetMailbox()->MailboxFree(pCmd);
			delete [] pCmd;
		while ( pEcMbSlave->m_pSoE->pCmds->RemoveFirstEntry(info) )
		{
			if ( info.pCmd )
				//m_pMaster->GetMailbox()->MailboxFree(info.pCmd);
				delete [] info.pCmd;
		}		
		safe_delete(pEcMbSlave->m_pSoE->pPend);
		safe_delete(pEcMbSlave->m_pSoE->pCmds);
		safe_delete(pEcMbSlave->m_pSoE);
	}
 
}

//\brief Sends an EtherCAT command request to read out the mailbox of the slave.
void MBoxReadFromSlaveEcMbSlave(PEcMbSlave pEcMbSlave)
{
	PEcSlave pEcSlave;

	pEcSlave = pEcMbSlave->m_pEcSlave;

	//check if there are any messages in the mailbox
	if( pEcSlave->m_cInitCmds == INITCMD_INACTIVE  )
		EcatCmdReqEcMaster( ( PEcMaster)pEcSlave->m_pMaster, 
							pEcSlave,
							ECSI_MBOX_RECV, 
							EC_CMD_TYPE_FPRD,
							pEcSlave->m_physAddr,
							pEcMbSlave->m_mbxIStart[pEcMbSlave->m_mbxIdx],
							pEcMbSlave->m_mbxILen[pEcMbSlave->m_mbxIdx], NULL);
}
 
///////////////////////////////////////////////////////////////////////////////
///\brief Executes the state machine of the EtherCAT slave.
int	StateMachineEcMbSlave(PEcMbSlave pEcMbSlave)
{	
	int bRet=0;
	if ( pEcMbSlave->m_cMbxCmds == INITCMD_INACTIVE )
	{	//no init commands active ( m_cInitCmds is also = INITCMD_INACTIVE )
                bRet = StateMachineEcSlave(pEcMbSlave->m_pEcSlave, 1);							   
		switch (pEcMbSlave->m_pEcSlave->m_currState)
		{
		case DEVICE_STATE_BOOTSTRAP:
			pEcMbSlave->m_mbxIdx	= 1;
			break;
		default:
			pEcMbSlave->m_mbxIdx	= 0;
		}
	}
	return bRet;
}

 
///////////////////////////////////////////////////////////////////////////////
///\brief Processes init commands that should be sent in this transition.
void StartInitCmdsEcMbSlave(PEcMbSlave pEcMbSlave, unsigned short transition)
{
	if ( pEcMbSlave->m_cMbxCmds == INITCMD_INACTIVE )
		pEcMbSlave->m_cMbxCmds = 0;
	else
	{				
		pEcMbSlave->m_cMbxCmds++;
	}
	
	//look for a CAN command that should be sent during this transition
	while ( pEcMbSlave->m_cMbxCmds < pEcMbSlave->m_nMbxCmds )
	{
		if ( pEcMbSlave->m_ppMbxCmds[ pEcMbSlave->m_cMbxCmds]->transition & transition )
			break;
		pEcMbSlave->m_cMbxCmds++;
	}
	
	if ( pEcMbSlave->m_cMbxCmds < pEcMbSlave->m_nMbxCmds )
	{	//found a CAN command for this transition
		
		//get a pointer to the current command description
		PEcMailboxCmdDesc p = pEcMbSlave->m_ppMbxCmds[pEcMbSlave->m_cMbxCmds];
		
		switch (p->protocol)
		{
		case ETHERCAT_MBOX_TYPE_CANOPEN:
			{
				//this is the amount of data that can be sent/received
				unsigned short nMaxData =  pEcMbSlave->m_mbxOLen[ pEcMbSlave->m_mbxIdx] - ETHERCAT_MIN_SDO_MBOX_LEN;
				unsigned short len = GetMBoxOutCmdLengthEcMbSlave(pEcMbSlave, ETHERCAT_MIN_SDO_MBOX_LEN);

				if ( p->timeout == 0 )
					Start(pEcMbSlave->m_tMbxCmds, 500);
				else
					Start(pEcMbSlave->m_tMbxCmds, p->timeout);
				pEcMbSlave->m_rMbxCmds	= p->retries;

						
				//intialize ethercat mailbox header
				ETHERCAT_MBOX_HEADER mbx ={0};
				mbx.Length	= ETHERCAT_CANOPEN_HEADER_LEN + ETHERCAT_SDO_HEADER_LEN;
				mbx.Type	= ETHERCAT_MBOX_TYPE_CANOPEN;
				
				//intialize CanOpen header
				ETHERCAT_CANOPEN_HEADER can = {0};
				can.Type		= ETHERCAT_CANOPEN_TYPE_SDOREQ;
				can.Number	= 0;
				
				//initialize Sdo header
				ETHERCAT_SDO_HEADER sdo = {0};

				void * pData=NULL;		
				pEcMbSlave->m_oMbxCmds		= 0;
				pEcMbSlave->m_actTransition= transition;
				//use command description to fill Sdo header
	
				sdo.Index			= p->coe.sdo.Index;	
				sdo.SubIndex		= p->coe.sdo.SubIndex;
				sdo.Idq.Ccs			= p->coe.sdo.Idq.Ccs;
				sdo.Idq.Complete	= p->coe.sdo.Idq.Complete;
				
				switch (sdo.Idq.Ccs)
				{			
				case SDO_CCS_INITIATE_DOWNLOAD:											
					if ( p->coe.sdo.Data > nMaxData )
					{
						//the data doen't fit into one ethercat frame
						//as a consequence one has to download 2 or more segments			
						sdo.Idq.Expedited	= 0;
						sdo.Idq.SizeInd	= 1;
						sdo.Idq.Size		= 0;
						sdo.Data				= p->coe.sdo.Data;
						pData					= &p->coe.data[0]; 
						//the actual amout of data without headers
						 pEcMbSlave->m_oMbxCmds			= nMaxData;
						//the actual amount of mailbox data to send in this ECAT-Frame including can and sdo header
						mbx.Length			+= nMaxData;
						//adjust length
						len					= GetMBoxOutCmdLengthEcMbSlave(pEcMbSlave,len+nMaxData);
					}
					else if (  p->coe.sdo.Data > sizeof(sdo.Data) )
					{
						//because length is greater than the maximal length in expedited mode
						sdo.Idq.Expedited	= 0;
						sdo.Idq.SizeInd	= 1;
						sdo.Idq.Size		= 0;
						sdo.Data				= p->coe.sdo.Data;
						pData					= &p->coe.data[0]; 
						pEcMbSlave->m_oMbxCmds			= (unsigned short)p->coe.sdo.Data;
						mbx.Length			+= (unsigned short)p->coe.sdo.Data;
						len					= GetMBoxOutCmdLengthEcMbSlave(pEcMbSlave,len+(unsigned short)p->coe.sdo.Data);
					}
					else if (  p->coe.sdo.Data > 0 )
					{				
						sdo.Idq.Expedited	= 1;
						sdo.Idq.SizeInd	= 1;				
						sdo.Idq.Size		= sizeof(sdo.Data)-p->coe.sdo.Data;
						memcpy(&sdo.Data, &p->coe.data[0], p->coe.sdo.Data);
						pEcMbSlave->m_oMbxCmds			= (unsigned short)p->coe.sdo.Data;
					}
					else
					{	
						sdo.Idq.Expedited	= 1;
						sdo.Idq.SizeInd	= 0;
						sdo.Idq.Size		= 0;
						pEcMbSlave->m_oMbxCmds			= 0;
					}
					//send command to the slave
					EcatCmdReqEcMaster(( PEcMaster)pEcMbSlave->m_pEcSlave->m_pMaster, pEcMbSlave->m_pEcSlave, transition, EC_CMD_TYPE_FPWR, pEcMbSlave->m_pEcSlave->m_physAddr, pEcMbSlave->m_mbxOStart[pEcMbSlave->m_mbxIdx], len,
						pData, 0, &mbx, NULL, &can, &sdo);
					break;
				case SDO_CCS_INITIATE_UPLOAD:
					break;		
				}
			}
			break;
		case ETHERCAT_MBOX_TYPE_SOE:
			{
				Start(pEcMbSlave->m_tMbxCmds, p->timeout!=0 ? p->timeout : 5000);
				pEcMbSlave->m_rMbxCmds	= p->retries;

				pEcMbSlave->m_actTransition			= transition;

				unsigned short			nData = (unsigned short)SIZEOF_EcMailboxCmdDescSoeData(p);
				ETHERCAT_MBOX_HEADER		mbx={0};
				ETHERCAT_SOE_HEADER		soe=p->soe.head;
				mbx.Type						= ETHERCAT_MBOX_TYPE_SOE;
				mbx.Length					= ETHERCAT_SOE_HEADER_LEN;

				if ( nData > GetSoEDataSizeEcMbSlave(pEcMbSlave, VG_OUT) )
				{
					pEcMbSlave->m_oMbxCmds			= GetSoEDataSizeEcMbSlave(pEcMbSlave,VG_OUT);
					soe.InComplete		= 1;
					soe.FragmentsLeft	= ((nData+GetSoEDataSizeEcMbSlave(pEcMbSlave,VG_OUT)-1) / GetSoEDataSizeEcMbSlave(pEcMbSlave,VG_OUT)) - 1;
					mbx.Length			+= GetSoEDataSizeEcMbSlave(pEcMbSlave,VG_OUT);
				}
				else
				{
					pEcMbSlave->m_oMbxCmds			= nData;
					mbx.Length			+= nData;
				}
				EcatMbxSendCmdReqEcMbSlave(pEcMbSlave, p->soe.data, &mbx, NULL, NULL, NULL, NULL, &soe);
			}
			break;
		}		
	}
	else
	{
		StartInitCmdsEcSlave(pEcMbSlave->m_pEcSlave, transition, 1 );
	}
}
 
///////////////////////////////////////////////////////////////////////////////
void	StopInitCmdsEcMbSlave(PEcMbSlave pEcMbSlave)
{
	pEcMbSlave->m_cMbxCmds = INITCMD_INACTIVE;
	pEcMbSlave->m_pEcSlave->m_cInitCmds = INITCMD_INACTIVE;
}

///////////////////////////////////////////////////////////////////////////////
int	InitCmdsActiveEcMbSlave(PEcMbSlave pEcMbSlave)
{
	return  pEcMbSlave->m_cMbxCmds != INITCMD_INACTIVE;
}

///////////////////////////////////////////////////////////////////////////////
///\brief Determines if the EtherCAT slave has an mailbox.
int	HasMailBoxEcMbSlave(PEcMbSlave pEcMbSlave)
{	
	return pEcMbSlave->m_mbxOLen[pEcMbSlave->m_mbxIdx] > 0 && pEcMbSlave->m_mbxILen[pEcMbSlave->m_mbxIdx] > 0;

}

///////////////////////////////////////////////////////////////////////////////
unsigned short	GetMBoxOutCmdLengthEcMbSlave(PEcMbSlave pEcMbSlave, unsigned short length)
{
	if ( pEcMbSlave->m_mbxOutShortSend )
		return length;
	else
		return pEcMbSlave->m_mbxOLen[pEcMbSlave->m_mbxIdx];
}

 

unsigned short	GetFoEDataSizeEcMbSlave(PEcMbSlave pEcMbSlave, int inOut)
{ 
	return ((inOut==VG_IN) ? pEcMbSlave->m_mbxILen[pEcMbSlave->m_mbxIdx] : pEcMbSlave->m_mbxOLen[pEcMbSlave->m_mbxIdx])-ETHERCAT_MBOX_HEADER_LEN-ETHERCAT_FOE_HEADER_LEN;
}

unsigned short	GetSoEDataSizeEcMbSlave(PEcMbSlave pEcMbSlave, int inOut)
{ 
	return ((inOut==VG_IN) ? pEcMbSlave->m_mbxILen[pEcMbSlave->m_mbxIdx] : pEcMbSlave->m_mbxOLen[pEcMbSlave->m_mbxIdx])-ETHERCAT_MBOX_HEADER_LEN-ETHERCAT_SOE_HEADER_LEN;
}
	
int	EcatMbxSendCmdReqEcMbSlave(PEcMbSlave pEcMbSlave,
					  void * pData,
					  PETHERCAT_MBOX_HEADER pMbox, 
					  unsigned int pEthernet,
					  PETHERCAT_CANOPEN_HEADER pCANopen,
					  PETHERCAT_SDO_HEADER pSDO,
					  PETHERCAT_FOE_HEADER pFoE,
					  PETHERCAT_SOE_HEADER pSoE
					 )
{
	return EcatCmdReqEcMaster(( PEcMaster)pEcMbSlave->m_pEcSlave->m_pMaster, pEcMbSlave->m_pEcSlave, ECSI_MBOX_SEND, EC_CMD_TYPE_FPWR, pEcMbSlave->m_pEcSlave->m_physAddr, pEcMbSlave->m_mbxOStart[pEcMbSlave->m_mbxIdx], 
			GetMBoxOutCmdLengthEcMbSlave( pEcMbSlave,ETHERCAT_MBOX_HEADER_LEN+pMbox->Length), pData, 0, pMbox, pEthernet, pCANopen, pSDO, pFoE, pSoE);
}
 



///////////////////////////////////////////////////////////////////////////////
///\brief Is called by the master to send a mailbox command to the slave.
long	MailboxReceiveFromMasterEcMbSlave(PEcSlave pEcSlave, PEcMailboxCmd pCmd )
{

	PEcMbSlave pEcMbSlave;

	pEcMbSlave = (PEcMbSlave)pEcSlave->m_pEcMbSlave;

	if ( pEcMbSlave->m_mbxOLen[pEcMbSlave->m_mbxIdx] == 0 )
		return ECERR_DEVICE_ERROR;	

	if ( IsCoECmdEcMbSlave( pEcMbSlave, pCmd) )
	{
		AddToOnTimerListEcMaster( (PEcMaster) pEcMbSlave->m_pEcSlave->m_pMaster, pEcMbSlave );
		if ( pEcMbSlave->m_pCoE->pPend->Add(pCmd) )
			return ECERR_NOERR;
		else
			return ECERR_DEVICE_INSERTMAILBOX;
	}
	else if ( IsFoECmdEcMbSlave(pEcMbSlave, pCmd) )
	{		
		AddToOnTimerListEcMaster( (PEcMaster)pEcMbSlave->m_pEcSlave->m_pMaster, pEcMbSlave );		
		if ( pEcMbSlave->m_pFoE->pPend->Add(pCmd) )
			return ECERR_NOERR;
		else
			return ECERR_DEVICE_INSERTMAILBOX;		
	}
	else if ( IsSoECmdEcMbSlave(pEcMbSlave,pCmd) )
	{
		AddToOnTimerListEcMaster( (PEcMaster)pEcMbSlave->m_pEcSlave->m_pMaster, pEcMbSlave);				
		if ( pEcMbSlave->m_pSoE->pPend->Add(pCmd) )
			return ECERR_NOERR;
		else
			return ECERR_DEVICE_INSERTMAILBOX;
	}
	else
		return ECERR_DEVICE_UNKNOWNMAILBOXCMD;
}



///////////////////////////////////////////////////////////////////////////////
//
int	CycleMBoxPollingEcMbSlave(PEcMbSlave pEcMbSlave)
{
	//bPoll is 1 if:
	//	1. m_cycleMbxPolling is set to 1
	//  or if state polliing  is used and
	//	2. the master is in init or in a transition IP(Init to Pre-Operational), SI or OI. 
	//		This is necessary because the cyclic commands are only sent from the state Pre-Operational on
	//		and mailbox commands might be sent for instance during the transition from init to Pre-Operational.			
	//	or
	//  3. the timeout m_tCyclicSend is elapsed. This timer elapses if the last call to CEcMaster::SetCyclicCmds has been
	//	more than 200ms ago. This enables mailbox communication even if CEcDevice::StartIo is not called cyclically.
	int bPoll = pEcMbSlave->m_cycleMbxPolling ||GetStateMachineEcMaster( (PEcMaster)pEcMbSlave->m_pEcSlave->m_pMaster) == DEVICE_STATE_INIT || CyclicTimeoutElapsedEcMaster( (PEcMaster)pEcMbSlave->m_pEcSlave->m_pMaster);
	
	if ( bPoll && pEcMbSlave->m_pEcSlave->m_currState == DEVICE_STATE_INIT && pEcMbSlave->m_cMbxCmds == INITCMD_INACTIVE )
	// don't poll if the current state of the slave device is in init and no mailbox init commands should be sent,
		bPoll = 0;

	return bPoll;
}

///////////////////////////////////////////////////////////////////////////////
///\brief This method is called cyclically by CEcMaster::OnTimer.
void	OnTimerEcMbSlave(PEcMbSlave pEcMbSlave,unsigned long nTickNow)
{			
	//The mailbox is read out(with MBoxReadFromSlave) every m_cycleMbxPollingTime Tick.
	//m_physAddr%m_cycleMbxPollingTime is used, so that individual MbSlaves read out the mailbox on different
	//ticks
	
 	if ( (int)(nTickNow%pEcMbSlave->m_cycleMbxPollingTime) == (int)(pEcMbSlave->m_pEcSlave->m_physAddr%pEcMbSlave->m_cycleMbxPollingTime) && CycleMBoxPollingEcMbSlave(pEcMbSlave) )
 		MBoxReadFromSlaveEcMbSlave(pEcMbSlave);	
	
	if ( pEcMbSlave->m_cMbxCmds != INITCMD_INACTIVE && pEcMbSlave->m_cMbxCmds < pEcMbSlave->m_nMbxCmds )
	{
                iChi=4;	
		if ( IsElapsed(pEcMbSlave->m_tMbxCmds) )
		{
			pEcMbSlave->m_pEcSlave->m_reqState		= pEcMbSlave->m_pEcSlave->m_currState;
			StopInitCmdsEcMbSlave(pEcMbSlave);
		}
	}
	
	if ( pEcMbSlave->m_pCoE )
		SendQueuedCoECmdsEcMbSlave(pEcMbSlave);
	if ( pEcMbSlave->m_pFoE )
		SendQueuedFoECmdEcMbSlave(pEcMbSlave);
	if ( pEcMbSlave->m_pSoE )
		SendQueuedSoECmdsEcMbSlave(pEcMbSlave);	
}

///////////////////////////////////////////////////////////////////////////////
int	SendFrameToSlaveEcMbSlave(PEcMbSlave pEcMbSlave, PETHERNET_FRAME pData, unsigned long nData)
{	// from switch to the EthetCAT Slave
	return STATUS_PENDING;
}



///////////////////////////////////////////////////////////////////////////////
void ClearCoESafeEcMbSlave(PEcMbSlave pEcMbSlave)
{
	if ( pEcMbSlave->m_pCoE )
	{
		if ( pEcMbSlave->m_pCoE->safe.pCmd )
			//m_pMaster->GetMailbox()->MailboxFree(pEcMbSlave->m_pCoE->safe.pCmd);
			delete [] pEcMbSlave->m_pCoE->safe.pCmd;
		if ( pEcMbSlave->m_pCoE->safe.pRetData )
			//m_pMaster->GetMailbox()->MailboxFree((PEcMailboxCmd)pEcMbSlave->m_pCoE->safe.pRetData);
			delete [] pEcMbSlave->m_pCoE->safe.pRetData;

		pEcMbSlave->m_pCoE->safe.bValid = 0;
		pEcMbSlave->m_pCoE->safe.pCmd   = NULL;				// MailboxAlloc
		pEcMbSlave->m_pCoE->safe.pRetData = NULL;			// MailboxAlloc
		pEcMbSlave->m_pCoE->safe.nRetData = 0;
		pEcMbSlave->m_pCoE->safe.nOffset = 0;
		pEcMbSlave->m_pCoE->safe.retry = 0;
		memset(&pEcMbSlave->m_pCoE->safe.sdo, 0, sizeof(pEcMbSlave->m_pCoE->safe.sdo));

		//memset(&pEcMbSlave->m_pCoE->safe, 0, sizeof(pEcMbSlave->m_pCoE->safe));

	}
}

///////////////////////////////////////////////////////////////////////////////
void ClearFoESafeEcMbSlave(PEcMbSlave pEcMbSlave)
{
	if ( pEcMbSlave->m_pFoE )
	{
		if ( pEcMbSlave->m_pFoE->safe.pCmd )
			//m_pMaster->GetMailbox()->MailboxFree(pEcMbSlave->m_pFoE->safe.pCmd);
			delete [] pEcMbSlave->m_pFoE->safe.pCmd;
		if ( pEcMbSlave->m_pFoE->safe.pRet )
			//m_pMaster->GetMailbox()->MailboxFree(pEcMbSlave->m_pFoE->safe.pRet);
			delete [] pEcMbSlave->m_pFoE->safe.pRet;
		if ( pEcMbSlave->m_pFoE->safe.pData )
			//m_pMaster->GetMailbox()->MailboxFree((PEcMailboxCmd)pEcMbSlave->m_pFoE->safe.pData);
			delete [] pEcMbSlave->m_pFoE->safe.pData;

		pEcMbSlave->m_pFoE->safe.bValid	=0;
		pEcMbSlave->m_pFoE->safe.bNewCmd=0;
		pEcMbSlave->m_pFoE->safe.bRead=0;
		pEcMbSlave->m_pFoE->safe.bDataPend=0;
		pEcMbSlave->m_pFoE->safe.bBusy=0;
		pEcMbSlave->m_pFoE->safe.bLastRead=0;
		pEcMbSlave->m_pFoE->safe.pRet=NULL;					// MailboxAlloc
		pEcMbSlave->m_pFoE->safe.pCmd=NULL;					// MailboxAlloc
		pEcMbSlave->m_pFoE->safe.pData=NULL;				// MailboxAlloc
		pEcMbSlave->m_pFoE->safe.nOffset=0;
		pEcMbSlave->m_pFoE->safe.retry=0;
		pEcMbSlave->m_pFoE->safe.nData=0;				
		pEcMbSlave->m_pFoE->safe.nPacketNo=0;				


		//memset(&pEcMbSlave->m_pFoE->safe, 0, sizeof(pEcMbSlave->m_pFoE->safe));
	}
}

///////////////////////////////////////////////////////////////////////////////
void ClearSoESafeEcMbSlave(PEcMbSlave pEcMbSlave)
{
	if ( pEcMbSlave->m_pSoE )
	{
		if ( pEcMbSlave->m_pSoE->safe.pCmd )
			//m_pMaster->GetMailbox()->MailboxFree(pEcMbSlave->m_pSoE->safe.pCmd);
			delete [] pEcMbSlave->m_pSoE->safe.pCmd;
		if ( pEcMbSlave->m_pSoE->safe.pRetData )
			//m_pMaster->GetMailbox()->MailboxFree((PEcMailboxCmd)pEcMbSlave->m_pSoE->safe.pRetData);
			delete [] pEcMbSlave->m_pSoE->safe.pRetData;

		pEcMbSlave->m_pSoE->safe.bValid=0;

		pEcMbSlave->m_pSoE->safe.pCmd=0;				// MailboxAlloc
		pEcMbSlave->m_pSoE->safe.pRetData=0;			// MailboxAlloc
		pEcMbSlave->m_pSoE->safe.nRetData=0;
		pEcMbSlave->m_pSoE->safe.retry=0;

		pEcMbSlave->m_pSoE->safe.nIDN=0;
		pEcMbSlave->m_pSoE->safe.nOffset=0;

		//memset(&pEcMbSlave->m_pSoE->safe, 0, sizeof(pEcMbSlave->m_pSoE->safe));
	}
}

///////////////////////////////////////////////////////////////////////////////
///\brief Sends if necessary an Sdo abort command to the mailbox slave and sends an error response to the mailbox client.
void	StopCoESafeRequestEcMbSlave(PEcMbSlave pEcMbSlave, unsigned long result, unsigned long abortCode, unsigned long nData, void * pData)
{
	if ( pEcMbSlave->m_pCoE == NULL )
		return;
	if ( abortCode != 0 )
	{
		ETHERCAT_MBOX_HEADER		mbx={0};
		ETHERCAT_CANOPEN_HEADER	can={0};
		ETHERCAT_SDO_HEADER		sdo={0};
		mbx.Type			= ETHERCAT_MBOX_TYPE_CANOPEN;
		mbx.Length		= ETHERCAT_CANOPEN_HEADER_LEN + ETHERCAT_SDO_HEADER_LEN;
		can.Type			= ETHERCAT_CANOPEN_TYPE_SDOREQ;
		sdo.Abt.Ccs		= SDO_CCS_ABORT_TRANSFER;
		sdo.Data			= abortCode;
		// abort to SDO server
		

		EcatMbxSendCmdReqEcMbSlave(pEcMbSlave, NULL, &mbx, NULL, &can, &sdo);
	}
	// error to Mailbox client
		
	MailboxResEcMbSlave( pEcMbSlave, pEcMbSlave->m_pCoE->safe.pCmd, result, nData, pData);			
	ClearCoESafeEcMbSlave( pEcMbSlave );
}
 

///////////////////////////////////////////////////////////////////////////////
#define	NOABORTSEND	0xffffffff
void	StopFoESafeRequestEcMbSlave(PEcMbSlave pEcMbSlave, unsigned long result, unsigned long errCode, unsigned long nErrText, PCHAR pErrText)
{
	if ( pEcMbSlave->m_pFoE == NULL )
		return;
	if ( errCode != NOABORTSEND )
	{
		ETHERCAT_MBOX_HEADER	mbx={0};
		ETHERCAT_FOE_HEADER	foe={0};
		mbx.Type				= ETHERCAT_MBOX_TYPE_FILEACCESS;
		mbx.Length				= (unsigned short)(ETHERCAT_FOE_HEADER_LEN + nErrText);
		foe.OpCode				= ECAT_FOE_OPCODE_ERR;
		foe.ErrorCode			= errCode;
		EcatMbxSendCmdReqEcMbSlave(pEcMbSlave, pErrText, &mbx, NULL, NULL, NULL, &foe);
	}
	
	if ( result != NOABORTSEND && pEcMbSlave->m_pFoE->safe.pCmd )
	{
		PEcMailboxCmd pCmd = ( PEcMailboxCmd ) pEcMbSlave->m_pFoE->safe.pCmd;
		MailboxResEcMbSlave( pEcMbSlave, pCmd, result, min(nErrText, pCmd->length), pErrText);
	}

	ClearFoESafeEcMbSlave( pEcMbSlave);
}
 
///////////////////////////////////////////////////////////////////////////////
///\brief Sends if necessary an Sdo abort command to the mailbox slave and sends an error response to the mailbox client.
void	StopSoESafeRequestEcMbSlave(PEcMbSlave pEcMbSlave, unsigned long result, unsigned long nData, void * pData)
{
	if ( pEcMbSlave->m_pSoE == NULL || !pEcMbSlave->m_pSoE->safe.bValid )
		return;

	// answer to mailbox client
	if ( pEcMbSlave->m_pSoE->safe.pCmd )
	{
		if ( pEcMbSlave->m_pSoE->safe.pCmd->type == EC_MAILBOX_CMD_UPLOAD )
		{			
			if ( pEcMbSlave->m_pSoE->safe.pCmd->length < nData )
			{
				nData = 0;
				pData = NULL;
				if ( result == ECERR_NOERR )
					result = ECERR_DEVICE_INVALIDSIZE;
			}
			MailboxResEcMbSlave( pEcMbSlave,pEcMbSlave->m_pSoE->safe.pCmd, result, nData, pData);
		}
		else
			MailboxResEcMbSlave( pEcMbSlave, pEcMbSlave->m_pSoE->safe.pCmd, result);
	}
	ClearSoESafeEcMbSlave( pEcMbSlave);
}

int MailboxResEcMbSlave(PEcMbSlave pEcMbSlave, PEcMailboxCmd pReq, unsigned long nResult, unsigned long cbLength, void * pData)
{
	switch ( pReq->type )
	{
	case EC_MAILBOX_CMD_UPLOAD:
	case EC_MAILBOX_CMD_READWRITE:		
		return MailboxUploadResEcMbSlave( pEcMbSlave, pReq, nResult, cbLength, pData);
		
	case EC_MAILBOX_CMD_DOWNLOAD:
		return MailboxDownloadResEcMbSlave( pEcMbSlave, pReq, nResult);
	default:
		return ECERR_DEVICE_INVALIDPARM;
	}
}

 
int MailboxUploadResEcMbSlave(PEcMbSlave pEcMbSlave, PEcMailboxCmd pReq, unsigned long nResult, unsigned long cbLength, void * pData)
{
	int ret = ECERR_NOERR;
	if ( cbLength != 0 && pData == NULL )
		return ECERR_DEVICE_INVALIDPARM;
	
	PEcMailboxCmd pRes = MailboxAlloc(sizeof(EcMailboxCmd) + cbLength );
	pRes->type = EC_MAILBOX_CMD_UPLOAD;
	if ( pRes == NULL )
	{
		pRes		= MailboxAlloc( sizeof(EcMailboxCmd) );
		ret			= nResult	= ECERR_DEVICE_NOMEMORY;
		cbLength	= 0;
	}
	
	if( pRes != NULL )
	{												
		memcpy(pRes, pReq, sizeof(EcMailboxCmd));
		pRes->length = cbLength ;
		pRes->result = nResult;
		memcpy(ENDOF(pRes), pData, cbLength);
		MailboxResponseEcMaster((PEcMaster)pEcMbSlave->m_pEcSlave->m_pMaster,  pRes); 
	}					
	return ret;
}
 
int MailboxDownloadResEcMbSlave(PEcMbSlave pEcMbSlave, PEcMailboxCmd pReq, unsigned long nResult)
{
	int ret = ECERR_NOERR;	
	PEcMailboxCmd pRes = MailboxAlloc(sizeof(EcMailboxCmd));
	if ( pRes == NULL )
	{
		ret = nResult = ECERR_DEVICE_NOMEMORY;
	}
	
	if( pRes != NULL )
	{												
		memcpy(pRes, pReq, sizeof(EcMailboxCmd));
		pRes->result = nResult;
		pRes->length = 0;
		MailboxResponseEcMaster((PEcMaster)pEcMbSlave->m_pEcSlave->m_pMaster,pRes); 
	}					
	return ret;
}



///////////////////////////////////////////////////////////////////////////////
///\brief Processes the response of an EtherCAT command.
void	EcatCmdResEcMbSlave(PEcMbSlave pEcMbSlave, unsigned long result, unsigned long invokeId, PETYPE_EC_HEADER pHead)
{
	PEcSlave pEcSlave;
	pEcSlave = pEcMbSlave->m_pEcSlave;

	unsigned int msg = EC_LOG_MSGTYPE_LOG|EC_LOG_MSGTYPE_ERROR;
	switch ( invokeId )
	{
	case ECSI_MBOX_SEND:
		if ( result == ECERR_NOERR )
		{
			unsigned short				wc		= ETYPE_EC_CMD_COUNT(pHead);
			PETHERCAT_MBOX_HEADER		pMBox	= (PETHERCAT_MBOX_HEADER)ENDOF(pHead);

			switch ( pMBox->Type )
			{
			case ETHERCAT_MBOX_TYPE_CANOPEN:
				ProcessCoEReturningRequestEcMbSlave(pEcMbSlave,wc, pMBox);
				break;
			case ETHERCAT_MBOX_TYPE_FILEACCESS:
				ProcessFoEReturningRequestEcMbSlave(pEcMbSlave, wc, pMBox);
				break;
			}
		}
		else if ( pEcMbSlave->m_cMbxCmds != INITCMD_INACTIVE ) // result != ECERR_NOERR
		{
			PEcMailboxCmdDesc p = pEcMbSlave->m_ppMbxCmds[pEcMbSlave->m_cMbxCmds];

			pEcSlave->m_reqState		= pEcSlave->m_currState;
			StopInitCmdsEcMbSlave(pEcMbSlave);
		}
		break;
	
	case ECSI_MBOX_RECV:
		if ( result == ECERR_NOERR )
		{
			PETHERCAT_MBOX_HEADER pMBox = (PETHERCAT_MBOX_HEADER)ENDOF(pHead);
			if ( ETYPE_EC_CMD_COUNT(pHead) == 1 )
			{	// succeeded
				switch ( pMBox->Type )
				{
				case ETHERCAT_MBOX_TYPE_CANOPEN:
					ProcessCoEResponseEcMbSlave(pEcMbSlave, pMBox);
					break;
				case ETHERCAT_MBOX_TYPE_FILEACCESS:
					ProcessFoEResponseEcMbSlave(pEcMbSlave, pMBox);
					break;
				case ETHERCAT_MBOX_TYPE_SOE:
					ProcessSoEResponseEcMbSlave(pEcMbSlave, pMBox);
					break;
				}
			}
		}
		break;
	default:
		EcatCmdResEcSlave( pEcSlave , result, invokeId, pHead, 1);
	}
}


 ///////////////////////////////////////////////////////////////////////////////
int IsCoECmdEcMbSlave( PEcMbSlave pEcMbSlave, PEcMailboxCmd pCmd )
{	
	return pEcMbSlave->m_pCoE && pCmd->cmdId >= MAILBOXCMD_CANOPEN_BEGIN && pCmd->cmdId <= MAILBOXCMD_CANOPEN_END;
}

///////////////////////////////////////////////////////////////////////////////
int IsFoECmdEcMbSlave( PEcMbSlave pEcMbSlave, PEcMailboxCmd pCmd )
{
	return pEcMbSlave->m_pFoE &&  pCmd->cmdId >= MAILBOXCMD_ECAT_FOE_BEGIN && pCmd->cmdId <= MAILBOXCMD_ECAT_FOE_END;
}

///////////////////////////////////////////////////////////////////////////////
int IsSoECmdEcMbSlave( PEcMbSlave pEcMbSlave, PEcMailboxCmd pCmd )
{
	
	return pEcMbSlave->m_pSoE && 
		( (pCmd->cmdId == MAILBOXCMD_ECAT_SOE && (pCmd->indexOffset&MAILBOXIOFFS_ECAT_SOE_ELEMENT_MASK) != 0) || 
		  (pCmd->indexOffset&MAILBOXIOFFS_ECAT_SOE_ELEMENT_MASK) == 0);		
}
 

///////////////////////////////////////////////////////////////////////////////
///\brief Sends the queued Sdo commands to the slave
void SendQueuedCoECmdsEcMbSlave( PEcMbSlave pEcMbSlave )
{
	if ( pEcMbSlave->m_pCoE == NULL )
		return;

	if ( pEcMbSlave->m_pCoE->safe.bValid )
	{	//still waiting for a response to a previous coe mailbox
                iChi = 5;
		if ( IsElapsed( pEcMbSlave->m_pCoE->safe.timeout) )
		{	//timeout has elapsed
			
			if ( pEcMbSlave->m_pCoE->safe.retry > 0 )
			{//at the moment safe.retry is always 0 because ECMAILBOX_DEFAULT_COE_RETRY is set to 0 			  	
			}
			else
			{//timeout has elapsed and retry = 0 -> send error response to mailbox and clear m_pCoe->safe				
				MailboxResEcMbSlave( pEcMbSlave, pEcMbSlave->m_pCoE->safe.pCmd, ECERR_DEVICE_TIMEOUT);				
				ClearCoESafeEcMbSlave( pEcMbSlave );
			}
		}
	}

	if ( pEcMbSlave->m_pCoE->safe.bValid == 0 && pEcMbSlave->m_pCoE->pPend->Remove(pEcMbSlave->m_pCoE->safe.pCmd) )
	{//not waiting for any command responses and a pending command exists	
		void * pData = NULL;
		
		//Add mailbox slave to OnTimer-List of the master
		AddToOnTimerListEcMaster((PEcMaster)pEcMbSlave->m_pEcSlave->m_pMaster,pEcMbSlave);
		unsigned short sendLength = ETHERCAT_MIN_SDO_MBOX_LEN;

		ETHERCAT_MBOX_HEADER		mbx={0};
		ETHERCAT_CANOPEN_HEADER	can={0};
		
		mbx.Type					= ETHERCAT_MBOX_TYPE_CANOPEN;
		pEcMbSlave->m_pCoE->safe.bValid	= 1;
		PEcMailboxCmd pCmd = pEcMbSlave->m_pCoE->safe.pCmd;
		
		if ( pCmd->cmdId == MAILBOXCMD_CANOPEN_SDO )
		{
			can.Type					= ETHERCAT_CANOPEN_TYPE_SDOREQ;
			mbx.Length				= ETHERCAT_CANOPEN_HEADER_LEN + ETHERCAT_SDO_HEADER_LEN;
		
			pEcMbSlave->m_pCoE->safe.sdo.Index			= pCmd->index;
			pEcMbSlave->m_pCoE->safe.sdo.SubIndex		= pCmd->subIndex;
				
			if ( pCmd->type == EC_MAILBOX_CMD_UPLOAD )
			{
				pEcMbSlave->m_pCoE->safe.sdo.Iuq.Ccs		= SDO_CCS_INITIATE_UPLOAD;
				pEcMbSlave->m_pCoE->safe.sdo.Iuq.Complete	= (pCmd->complete == 1);
			}
			else if ( pCmd->type == EC_MAILBOX_CMD_DOWNLOAD )
			{
				unsigned short nMaxData = pEcMbSlave->m_mbxOLen[pEcMbSlave->m_mbxIdx] - ETHERCAT_MIN_SDO_MBOX_LEN;			

				pEcMbSlave->m_pCoE->safe.sdo.Idq.Ccs = SDO_CCS_INITIATE_DOWNLOAD;			
				if ( pCmd->length > nMaxData )
				{
					pEcMbSlave->m_pCoE->safe.sdo.Idq.Expedited	= 0;
					pEcMbSlave->m_pCoE->safe.sdo.Idq.SizeInd	= 1;
					pEcMbSlave->m_pCoE->safe.sdo.Idq.Size		= 0;
					pEcMbSlave->m_pCoE->safe.sdo.Data			= pCmd->length;
					pData										= ENDOF(pCmd);
					mbx.Length									+= nMaxData;					
					pEcMbSlave->m_pCoE->safe.nOffset			= nMaxData;
				}
				else if (  pCmd->length > sizeof(pEcMbSlave->m_pCoE->safe.sdo.Data) ) // pCmd->length > 4
				{
					pEcMbSlave->m_pCoE->safe.sdo.Idq.Expedited	= 0;
					pEcMbSlave->m_pCoE->safe.sdo.Idq.SizeInd	= 1;
					pEcMbSlave->m_pCoE->safe.sdo.Idq.Size		= 0;
					pEcMbSlave->m_pCoE->safe.sdo.Data			= pCmd->length;	
					pData										= ENDOF(pCmd);
					mbx.Length								   += (unsigned short) pCmd->length;					
					pEcMbSlave->m_pCoE->safe.nOffset			= (unsigned short) pCmd->length;
				}
				else if (  pCmd->length > 0 )
				{
					pEcMbSlave->m_pCoE->safe.sdo.Idq.Expedited	= 1;
					pEcMbSlave->m_pCoE->safe.sdo.Idq.SizeInd	= 1;
					pEcMbSlave->m_pCoE->safe.sdo.Idq.Size		= sizeof(pEcMbSlave->m_pCoE->safe.sdo.Data)- pCmd->length;
					memcpy(&pEcMbSlave->m_pCoE->safe.sdo.Data, ENDOF( pEcMbSlave->m_pCoE->safe.pCmd),  pCmd->length);
					pEcMbSlave->m_pCoE->safe.nOffset			= (unsigned short)pCmd->length;
				}
				else
				{
					pEcMbSlave->m_pCoE->safe.sdo.Idq.Expedited	= 1;
					pEcMbSlave->m_pCoE->safe.sdo.Idq.SizeInd	= 0;
					pEcMbSlave->m_pCoE->safe.sdo.Idq.Size		= 0;
					pEcMbSlave->m_pCoE->safe.nOffset			=  (unsigned short)pCmd->length;
				}
			}
			pEcMbSlave->m_pCoE->safe.retry	= ECMAILBOX_DEFAULT_COE_RETRY;

			Start(pEcMbSlave->m_pCoE->safe.timeout, ECMAILBOX_DEFAULT_COE_TMOUT);

			//send to slave
			EcatMbxSendCmdReqEcMbSlave(pEcMbSlave, pData, &mbx, NULL, &can, &pEcMbSlave->m_pCoE->safe.sdo);
			
		}
		else if ( pCmd->cmdId == MAILBOXCMD_CANOPEN_SDO_INFO_LIST && pCmd->type == EC_MAILBOX_CMD_UPLOAD )
		{
			can.Type					= ETHERCAT_CANOPEN_TYPE_SDOINFO;
			mbx.Length					= ETHERCAT_CANOPEN_HEADER_LEN + ETHERCAT_SDO_INFO_LISTREQ_LEN;
			ETHERCAT_SDO_INFO_HEADER info={0};
			info.OpCode					= ECAT_COE_INFO_OPCODE_LIST_Q;
			info.List.ListType			= pCmd->index;

			pEcMbSlave->m_pCoE->safe.retry		= ECMAILBOX_DEFAULT_COE_RETRY;
			Start(pEcMbSlave->m_pCoE->safe.timeout,ECMAILBOX_DEFAULT_COE_TMOUT);

			EcatMbxSendCmdReqEcMbSlave(pEcMbSlave, &info, &mbx, NULL, &can);
		}
		else if ( pCmd->cmdId == MAILBOXCMD_CANOPEN_SDO_INFO_OBJ && pCmd->type == EC_MAILBOX_CMD_UPLOAD )
		{
			can.Type					= ETHERCAT_CANOPEN_TYPE_SDOINFO;
	 		mbx.Length					= ETHERCAT_CANOPEN_HEADER_LEN + ETHERCAT_SDO_INFO_OBJREQ_LEN;
			ETHERCAT_SDO_INFO_HEADER info={0};
			info.OpCode					= ECAT_COE_INFO_OPCODE_OBJ_Q;
			info.Obj.Index				= pCmd->index;

			pEcMbSlave->m_pCoE->safe.retry		= ECMAILBOX_DEFAULT_COE_RETRY;
			Start(pEcMbSlave->m_pCoE->safe.timeout, ECMAILBOX_DEFAULT_COE_TMOUT);

			EcatMbxSendCmdReqEcMbSlave(pEcMbSlave, &info, &mbx, NULL, &can);
		}
		else if ( pCmd->cmdId == MAILBOXCMD_CANOPEN_SDO_INFO_ENTRY && pCmd->type == EC_MAILBOX_CMD_UPLOAD )
		{
			can.Type						= ETHERCAT_CANOPEN_TYPE_SDOINFO;
	 		mbx.Length					= ETHERCAT_CANOPEN_HEADER_LEN + ETHERCAT_SDO_INFO_ENTRYREQ_LEN;
			ETHERCAT_SDO_INFO_HEADER info={0};
			info.OpCode					= ECAT_COE_INFO_OPCODE_ENTRY_Q;
			info.Entry.Index			= pCmd->index;
			info.Entry.SubIdx			= pCmd->subIndex;
			info.Entry.ValueInfo		= pCmd->valueInfo;

			pEcMbSlave->m_pCoE->safe.retry		= ECMAILBOX_DEFAULT_COE_RETRY;
			Start(pEcMbSlave->m_pCoE->safe.timeout, ECMAILBOX_DEFAULT_COE_TMOUT);

			EcatMbxSendCmdReqEcMbSlave(pEcMbSlave, &info, &mbx, NULL, &can);
		}
		else
			StopCoESafeRequestEcMbSlave(pEcMbSlave, ECERR_DEVICE_INVALIDCMD);
	}	
}
 

 
///////////////////////////////////////////////////////////////////////////////
void	SendQueuedFoECmdEcMbSlave( PEcMbSlave pEcMbSlave)
{	
	if ( pEcMbSlave->m_pFoE == NULL )
		return;

	unsigned int msg = EC_LOG_MSGTYPE_LOG|EC_LOG_MSGTYPE_ERROR;
	if ( pEcMbSlave->m_pFoE->safe.bValid )
	{
		if ( pEcMbSlave->m_pFoE->safe.bBusy )
		{
                        iChi = 6;
                	if ( IsElapsed(pEcMbSlave->m_pFoE->safe.busy) )
			{
				pEcMbSlave->m_pFoE->safe.bBusy		= 0;

				PEcMailboxCmdRW pCmd = pEcMbSlave->m_pFoE->safe.pCmd;
				unsigned long nData = min(GetFoEDataSizeEcMbSlave(pEcMbSlave,VG_OUT), pCmd->writeLength-pEcMbSlave->m_pFoE->safe.nOffset);
				// send data packet again
				ETHERCAT_MBOX_HEADER		mbx={0};
				ETHERCAT_FOE_HEADER		foe={0};
				mbx.Type					= ETHERCAT_MBOX_TYPE_FILEACCESS;
				mbx.Length					= (unsigned short)(ETHERCAT_FOE_HEADER_LEN + nData);
				foe.OpCode					= ECAT_FOE_OPCODE_DATA;
				foe.PacketNo				= pEcMbSlave->m_pFoE->safe.nPacketNo;
				pEcMbSlave->m_pFoE->safe.retry		= ECMAILBOX_DEFAULT_FOE_RETRY;
				Start(pEcMbSlave->m_pFoE->safe.timeout, ECMAILBOX_DEFAULT_FOE_TMOUT);
 
				unsigned char*	pByte				= (unsigned char*)ENDOF(pCmd);
				EcatMbxSendCmdReqEcMbSlave(pEcMbSlave, &pByte[pEcMbSlave->m_pFoE->safe.nOffset], &mbx, NULL, NULL, NULL, &foe);
			}
		}
		else
		{
                        iChi = 7;
			if ( IsElapsed(pEcMbSlave->m_pFoE->safe.timeout) )
			{
				if ( pEcMbSlave->m_pFoE->safe.pCmd )
					MailboxResEcMbSlave(pEcMbSlave, (PEcMailboxCmd)pEcMbSlave->m_pFoE->safe.pCmd, ECERR_DEVICE_TIMEOUT);
				ClearFoESafeEcMbSlave(pEcMbSlave);
			}
		}
	}
	PEcMailboxCmd pCmd;

	if (pEcMbSlave->m_pFoE->pPend->Remove(pCmd) )
	{
		long result = ECERR_NOERR;					
		PEcMailboxCmdRW pFoeCmd = (PEcMailboxCmdRW)pCmd;
		
		switch (pFoeCmd->cmdId)
		{
		case MAILBOXCMD_ECAT_FOE_FOPENREAD:
			if ( pFoeCmd->length <= sizeof(pEcMbSlave->m_pFoE->fileHnd) )
				result =  ECERR_DEVICE_INVALIDSIZE;
			else if ( pEcMbSlave->m_pFoE->safe.bValid )
				result =  ECERR_DEVICE_BUSY;
			else
			{
				pEcMbSlave->m_pFoE->safe.pData = (unsigned char *)MailboxAlloc(GetFoEDataSizeEcMbSlave(pEcMbSlave, VG_IN));
				pEcMbSlave->m_pFoE->safe.nData = 0;
				if ( pEcMbSlave->m_pFoE->safe.pData )
				{
					pEcMbSlave->m_pFoE->fileHnd++;
					pEcMbSlave->m_pFoE->safe.bValid	= 1;
					pEcMbSlave->m_pFoE->safe.bNewCmd = 1;
					pEcMbSlave->m_pFoE->safe.pCmd		= pFoeCmd;
					Start(pEcMbSlave->m_pFoE->safe.timeout, ECMAILBOX_DEFAULT_FOE_TMOUT);
				}
				else
					result = ECERR_DEVICE_NOMEMORY;
			}
			break;
		case MAILBOXCMD_ECAT_FOE_FOPENWRITE:
			if ( pFoeCmd->length <= sizeof(pEcMbSlave->m_pFoE->fileHnd) )
				result = ECERR_DEVICE_INVALIDSIZE;
			else if ( pEcMbSlave->m_pFoE->safe.bValid )
				result = ECERR_DEVICE_BUSY;
			else
			{
				pEcMbSlave->m_pFoE->fileHnd++;
				pEcMbSlave->m_pFoE->safe.bValid	= 1;
				pEcMbSlave->m_pFoE->safe.bNewCmd = 1;
				pEcMbSlave->m_pFoE->safe.pCmd		= pFoeCmd;
				Start(pEcMbSlave->m_pFoE->safe.timeout, ECMAILBOX_DEFAULT_FOE_TMOUT);
			}
			break;
		case MAILBOXCMD_ECAT_FOE_PROGRESSINFO:
			if ( pFoeCmd->writeLength != 0 || pFoeCmd->length < ETHERCAT_FOE_BUSY_INFO_LEN )			
			{
				result = ECERR_DEVICE_INVALIDSIZE;
				break;
			}
			if ( pFoeCmd->indexOffset == pEcMbSlave->m_pFoE->fileHnd )
			{
				if( MailboxUploadResEcMbSlave(pEcMbSlave, pCmd, result, sizeof(pEcMbSlave->m_pFoE->safe.busyInfo), &pEcMbSlave->m_pFoE->safe.busyInfo) == ECERR_NOERR )
				{
					MailboxFree(pCmd);	
					pCmd = NULL;
				}
			}
			else
				result = ECERR_DEVICE_INVALIDOFFSET;
			break;
		case MAILBOXCMD_ECAT_FOE_FREAD:
			if ( pFoeCmd->writeLength != 0 )
			{
				result = ECERR_DEVICE_INVALIDSIZE;
				break;
			}
			if ( pFoeCmd->indexOffset == pEcMbSlave->m_pFoE->fileHnd )
			{
				if ( pEcMbSlave->m_pFoE->safe.bValid )
				{
					if ( pEcMbSlave->m_pFoE->safe.pRet == NULL )
					{
						pEcMbSlave->m_pFoE->safe.pRet = MailboxAlloc(sizeof(EcMailboxCmd) + pFoeCmd->length);						
						if ( pEcMbSlave->m_pFoE->safe.pRet )
						{
							memcpy(pEcMbSlave->m_pFoE->safe.pRet, pFoeCmd, sizeof(EcMailboxCmd));
							pEcMbSlave->m_pFoE->safe.pRet->length		= 0;
							pEcMbSlave->m_pFoE->safe.pRet->result		= ECERR_NOERR;	
							pEcMbSlave->m_pFoE->safe.nOffset		= 0;
							pEcMbSlave->m_pFoE->safe.bNewCmd		= 1;
							pEcMbSlave->m_pFoE->safe.pCmd			= pFoeCmd;
						}
						else
							result = ECERR_DEVICE_NOMEMORY;
					}
					else
						result = ECERR_DEVICE_SYNTAX;
				}
				else
					result = ECERR_DEVICE_BUSY;
			}
			else
				result = ECERR_DEVICE_INVALIDOFFSET;
			break;
		case MAILBOXCMD_ECAT_FOE_FCLOSE:
			if ( pFoeCmd->writeLength != 0 )
			{
				result = ECERR_DEVICE_INVALIDSIZE;
				break;
			}
			// no break;
		case MAILBOXCMD_ECAT_FOE_FWRITE:
			if ( pFoeCmd->indexOffset == pEcMbSlave->m_pFoE->fileHnd )
			{
				if ( pEcMbSlave->m_pFoE->safe.bValid )
				{
					if ( pEcMbSlave->m_pFoE->safe.pCmd == NULL )
					{
						pEcMbSlave->m_pFoE->safe.nOffset		= 0;
						pEcMbSlave->m_pFoE->safe.bNewCmd		= 1;
						pEcMbSlave->m_pFoE->safe.pCmd			= pFoeCmd;
					}
					else
					{	// pending data exits						
						unsigned long nPend = pEcMbSlave->m_pFoE->safe.pCmd->writeLength- pEcMbSlave->m_pFoE->safe.nOffset;
												
						// alloc complete new command and assemble it from the pending and the new one
						PEcMailboxCmdRW pCmdNew = (PEcMailboxCmdRW)MailboxAlloc(sizeof(EcMailboxCmdRW)+ nPend + pFoeCmd->writeLength);
						if ( pCmdNew )
						{
							unsigned char* pTmp = (unsigned char*)pCmdNew;
							// copy header from new cmd
							memcpy(pTmp, pFoeCmd, sizeof(EcMailboxCmdRW));
							pTmp += sizeof(EcMailboxCmdRW);
							// copy pending data
							unsigned char*	pByteTmp = (unsigned char*)(ENDOF(pEcMbSlave->m_pFoE->safe.pCmd));
							memcpy(pTmp, &pByteTmp[pEcMbSlave->m_pFoE->safe.nOffset], nPend);
							pTmp += nPend;
							// copy data from	new cmd
							if ( pFoeCmd->writeLength > 0 )
								memcpy(pTmp, ENDOF(pFoeCmd), pFoeCmd->writeLength);
							pTmp += pFoeCmd->writeLength;
							
							MailboxFree(pCmd);
							pCmd = NULL;
							MailboxFree((PEcMailboxCmd)pEcMbSlave->m_pFoE->safe.pCmd);
							pEcMbSlave->m_pFoE->safe.pCmd = NULL;

							pFoeCmd = pCmdNew;							
							pFoeCmd->writeLength	+= nPend;
	
							pEcMbSlave->m_pFoE->safe.nOffset		= 0;
							pEcMbSlave->m_pFoE->safe.bNewCmd		= 1;
							pEcMbSlave->m_pFoE->safe.pCmd			= pFoeCmd;
						}
						else
							result = ECERR_DEVICE_NOMEMORY;
					}
				}
				else
					result = ECERR_DEVICE_BUSY;
			}
			else
				result = ECERR_DEVICE_INVALIDOFFSET;
			break;
		default:
			result = ECERR_DEVICE_INVALIDCMD;
		}

		if ( result != ECERR_NOERR )
		{
			if ( MailboxResEcMbSlave(pEcMbSlave, pCmd, result) == ECERR_NOERR )
			{

				MailboxFree(pCmd);
				pCmd = NULL;
			}		
		}		
	}
	if ( pEcMbSlave->m_pFoE->safe.bValid == 1 && pEcMbSlave->m_pFoE->safe.bNewCmd )
	{	// only when new PCmd arrived
		pEcMbSlave->m_pFoE->safe.bNewCmd = 0;
		PEcMailboxCmdRW pCmd = pEcMbSlave->m_pFoE->safe.pCmd;
		
		ETHERCAT_MBOX_HEADER		mbx={0};
		ETHERCAT_FOE_HEADER		foe={0};
		mbx.Type						= ETHERCAT_MBOX_TYPE_FILEACCESS;
		pEcMbSlave->m_pFoE->safe.retry		= ECMAILBOX_DEFAULT_FOE_RETRY;
		Start(pEcMbSlave->m_pFoE->safe.timeout, ECMAILBOX_DEFAULT_FOE_TMOUT);

		switch (pCmd->cmdId)
		{
		case MAILBOXCMD_ECAT_FOE_FOPENREAD:
		case MAILBOXCMD_ECAT_FOE_FOPENWRITE:
			{
				mbx.Length					= (unsigned short)(ETHERCAT_FOE_HEADER_LEN + pCmd->writeLength);
				foe.OpCode					= (pCmd->cmdId==MAILBOXCMD_ECAT_FOE_FOPENREAD) ? ECAT_FOE_OPCODE_RRQ : ECAT_FOE_OPCODE_WRQ;
				foe.Password				= pCmd->indexOffset;
				pEcMbSlave->m_pFoE->safe.bRead		= (pCmd->cmdId==MAILBOXCMD_ECAT_FOE_FOPENREAD);
				pEcMbSlave->m_pFoE->safe.nPacketNo	= (pCmd->cmdId==MAILBOXCMD_ECAT_FOE_FOPENREAD) ? 1 : 0;

				EcatMbxSendCmdReqEcMbSlave(pEcMbSlave, ENDOF(pCmd), &mbx, NULL, NULL, NULL, &foe);
			}
			break;
		case MAILBOXCMD_ECAT_FOE_FCLOSE:
			if ( pEcMbSlave->m_pFoE->safe.bRead )
				StopFoESafeRequestEcMbSlave(pEcMbSlave, ECERR_NOERR, NOABORTSEND);
			else
			{	// send data packet
				mbx.Length			= ETHERCAT_FOE_HEADER_LEN;
				unsigned char*	 pByte		= NULL;
				if ( pEcMbSlave->m_pFoE->safe.pCmd )
				{	// pending data from last MAILBOXCMD_ECAT_FOE_FWRITE cmd
					unsigned long nPend = pCmd->writeLength-pEcMbSlave->m_pFoE->safe.nOffset;

					pByte			= (unsigned char*)ENDOF(pCmd);
					mbx.Length	+= (unsigned short)nPend;
				}
				foe.OpCode					= ECAT_FOE_OPCODE_DATA;
				foe.PacketNo				= pEcMbSlave->m_pFoE->safe.nPacketNo;
				pEcMbSlave->m_pFoE->safe.bDataPend	= 1;

				EcatMbxSendCmdReqEcMbSlave(pEcMbSlave, pByte, &mbx, NULL, NULL, NULL, &foe);
			}
			break;
		case MAILBOXCMD_ECAT_FOE_FREAD:
			if ( pEcMbSlave->m_pFoE->safe.nData > 0 )
			{
				PEcMailboxCmd pCmdRet = pEcMbSlave->m_pFoE->safe.pRet;
				unsigned long nAct = min(pEcMbSlave->m_pFoE->safe.nData, pCmd->length);				
				memcpy(ENDOF(pCmdRet), pEcMbSlave->m_pFoE->safe.pData, nAct);
				pEcMbSlave->m_pFoE->safe.nOffset	= nAct;
				pEcMbSlave->m_pFoE->safe.nData	-= (unsigned short)nAct;

				pCmdRet->length						+= nAct;				

				if ( pEcMbSlave->m_pFoE->safe.nData == 0 )
				{
					mbx.Length					= ETHERCAT_FOE_HEADER_LEN;
					foe.OpCode					= ECAT_FOE_OPCODE_ACK;
					foe.PacketNo				= pEcMbSlave->m_pFoE->safe.nPacketNo;
					pEcMbSlave->m_pFoE->safe.nPacketNo++;

					EcatMbxSendCmdReqEcMbSlave(pEcMbSlave, NULL, &mbx, NULL, NULL, NULL, &foe);
					if ( pEcMbSlave->m_pFoE->safe.bLastRead )
					{						
						MailboxResponseEcMaster( (PEcMaster)pEcMbSlave->m_pEcSlave->m_pMaster, pEcMbSlave->m_pFoE->safe.pRet);
						pEcMbSlave->m_pFoE->safe.nOffset		= 0;
						pEcMbSlave->m_pFoE->safe.pRet			= NULL;
						pEcMbSlave->m_pFoE->safe.bLastRead	= 0;
					}
				}
				else
				{
					memmove(pEcMbSlave->m_pFoE->safe.pData, &pEcMbSlave->m_pFoE->safe.pData[nAct], pEcMbSlave->m_pFoE->safe.nData);					
					MailboxResponseEcMaster( (PEcMaster)pEcMbSlave->m_pEcSlave->m_pMaster,pEcMbSlave->m_pFoE->safe.pRet);
					pEcMbSlave->m_pFoE->safe.nOffset	= 0;
					pEcMbSlave->m_pFoE->safe.pRet		= NULL;
				}
			}
			break;
		case MAILBOXCMD_ECAT_FOE_FWRITE:
			if ( pCmd->writeLength >= GetFoEDataSizeEcMbSlave(pEcMbSlave,VG_OUT)+pEcMbSlave->m_pFoE->safe.nOffset )
			{	// send data packet
				unsigned char*							pByte = (unsigned char*)ENDOF(pCmd);
				mbx.Length					= (unsigned short)(ETHERCAT_FOE_HEADER_LEN + GetFoEDataSizeEcMbSlave(pEcMbSlave,VG_OUT));
				foe.OpCode					= ECAT_FOE_OPCODE_DATA;
				foe.PacketNo				= pEcMbSlave->m_pFoE->safe.nPacketNo;
				pEcMbSlave->m_pFoE->safe.bDataPend	= 1;

				EcatMbxSendCmdReqEcMbSlave(pEcMbSlave, &pByte[pEcMbSlave->m_pFoE->safe.nOffset], &mbx, NULL, NULL, NULL, &foe);
			}
			else
			{	// need more data or an ADSIOFFS_ECAT_FOE_FCLOSE
				if ( MailboxResEcMbSlave(pEcMbSlave, (PEcMailboxCmd)pEcMbSlave->m_pFoE->safe.pCmd, ECERR_NOERR, 0, NULL) != ECERR_NOERR )
					StopFoESafeRequestEcMbSlave(pEcMbSlave, NOABORTSEND, ECAT_FOE_ERRCODE_DISKFULL);
			}
			break;
		}		
	}
}
 
///////////////////////////////////////////////////////////////////////////////
void	SendQueuedSoECmdsEcMbSlave( PEcMbSlave pEcMbSlave )
{
	if ( pEcMbSlave->m_pSoE == NULL )
		return;
	if ( pEcMbSlave->m_pSoE->safe.bValid )
	{
                iChi = 8;
		if ( IsElapsed(pEcMbSlave->m_pSoE->safe.timeout) )
		{
			if ( pEcMbSlave->m_pSoE->safe.retry > 0 )
			{
				pEcMbSlave->m_pSoE->safe.retry--;
				Start(pEcMbSlave->m_pSoE->safe.timeout, ECMAILBOX_DEFAULT_SOE_TMOUT);
//				QueueAmsCmd(m_pAoE->safe.pCmd);
			}
			else
			{
				if ( pEcMbSlave->m_pSoE->safe.pCmd->type == EC_MAILBOX_CMD_UPLOAD )
					MailboxResEcMbSlave( pEcMbSlave, pEcMbSlave->m_pSoE->safe.pCmd, ECERR_DEVICE_TIMEOUT, 0, NULL);
				else
					MailboxResEcMbSlave( pEcMbSlave, pEcMbSlave->m_pSoE->safe.pCmd, ECERR_DEVICE_TIMEOUT);

				ClearSoESafeEcMbSlave( pEcMbSlave);
			}
		}
	}
	else if ( !InitCmdsActiveEcMbSlave( pEcMbSlave ) )
	{
		PEcMailboxCmd pCmd;
		if ( pEcMbSlave->m_pSoE->pPend->Remove(pCmd) )
		{
			ETHERCAT_MBOX_HEADER			mbx={0};
			mbx.Type						= ETHERCAT_MBOX_TYPE_SOE;
			mbx.Length						= ETHERCAT_SOE_HEADER_LEN;
		//memset(&pEcMbSlave->m_pSoE->safe, 0, sizeof(pEcMbSlave->m_pSoE->safe));
			pEcMbSlave->m_pSoE->safe.bValid=0;

			pEcMbSlave->m_pSoE->safe.pCmd=0;				// MailboxAlloc
			pEcMbSlave->m_pSoE->safe.pRetData=0;			// MailboxAlloc
			pEcMbSlave->m_pSoE->safe.nRetData=0;
			pEcMbSlave->m_pSoE->safe.retry=0;

			pEcMbSlave->m_pSoE->safe.nIDN=0;
			pEcMbSlave->m_pSoE->safe.nOffset=0;
			pEcMbSlave->m_pSoE->safe.bValid			= 1;
			pEcMbSlave->m_pSoE->safe.pCmd				= pCmd;			
			PETHERCAT_SOE_MAILBOX_IOFFS pIO	= (PETHERCAT_SOE_MAILBOX_IOFFS)&pEcMbSlave->m_pSoE->safe.pCmd->indexOffset;
			pEcMbSlave->m_pSoE->safe.nIDN				= pIO->IDN;
			pEcMbSlave->m_pSoE->safe.soe.Elements	= pIO->Elements;
			pEcMbSlave->m_pSoE->safe.soe.DriveNo	= pIO->DriveNo;

			void *  pData = NULL;
			if ( pEcMbSlave->m_pSoE->safe.pCmd->type == EC_MAILBOX_CMD_UPLOAD )
			{
				pEcMbSlave->m_pSoE->safe.soe.IDN		= pEcMbSlave->m_pSoE->safe.nIDN;
				pEcMbSlave->m_pSoE->safe.soe.OpCode		= ECAT_SOE_OPCODE_RRQ;
			}
			else
			{
				pEcMbSlave->m_pSoE->safe.soe.OpCode		= ECAT_SOE_OPCODE_WRQ;
				if ( pEcMbSlave->m_pSoE->safe.pCmd->length <= GetSoEDataSizeEcMbSlave( pEcMbSlave,VG_OUT) )
				{
					pEcMbSlave->m_pSoE->safe.soe.IDN	= pEcMbSlave->m_pSoE->safe.nIDN;
					mbx.Length								+= (unsigned short)pEcMbSlave->m_pSoE->safe.pCmd->length;
				}
				else
				{
					pEcMbSlave->m_pSoE->safe.soe.InComplete		= 1;
					pEcMbSlave->m_pSoE->safe.soe.FragmentsLeft	= (unsigned short)((pEcMbSlave->m_pSoE->safe.pCmd->length+GetSoEDataSizeEcMbSlave( pEcMbSlave,VG_OUT)-1) / GetSoEDataSizeEcMbSlave( pEcMbSlave,VG_OUT)) - 1;
					mbx.Length								+= GetSoEDataSizeEcMbSlave( pEcMbSlave,VG_OUT);
				}
				pData								= ENDOF(pEcMbSlave->m_pSoE->safe.pCmd);
			}
			pEcMbSlave->m_pSoE->safe.retry		= ECMAILBOX_DEFAULT_SOE_RETRY;
			Start(pEcMbSlave->m_pSoE->safe.timeout, ECMAILBOX_DEFAULT_SOE_TMOUT);

			EcatMbxSendCmdReqEcMbSlave( pEcMbSlave, pData, &mbx, NULL, NULL, NULL, NULL, &pEcMbSlave->m_pSoE->safe.soe);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
void	ProcessCoEReturningRequestEcMbSlave( PEcMbSlave pEcMbSlave, unsigned short wc, PETHERCAT_MBOX_HEADER pMBox)
{
	if ( pEcMbSlave->m_pCoE == NULL )
		return;
	if ( wc == 1 )
	{
		if ( pEcMbSlave->m_pCoE->safe.bValid )
			pEcMbSlave->m_pCoE->safe.retry = 0;

		// trigger mailbox polling -> server may want to respond
		if ( CycleMBoxPollingEcMbSlave( pEcMbSlave) )
			MBoxReadFromSlaveEcMbSlave( pEcMbSlave);
	}
}


///////////////////////////////////////////////////////////////////////////////
void	ProcessFoEReturningRequestEcMbSlave( PEcMbSlave pEcMbSlave, unsigned short wc, PETHERCAT_MBOX_HEADER pMBox)
{
	if ( pEcMbSlave->m_pFoE == NULL )
		return;
	if ( wc == 1 )
	{
		if ( pEcMbSlave->m_pFoE->safe.bValid )
			pEcMbSlave->m_pFoE->safe.retry = 0;

		// trigger mailbox polling -> server may want to respond
		if ( CycleMBoxPollingEcMbSlave( pEcMbSlave) )
			MBoxReadFromSlaveEcMbSlave( pEcMbSlave);
	}
}


///////////////////////////////////////////////////////////////////////////////
void	ProcessSoEReturningRequestEcMbSlave( PEcMbSlave pEcMbSlave, unsigned short wc, PETHERCAT_MBOX_HEADER pMBox)
{
	if ( pEcMbSlave->m_pSoE == NULL )
		return;
	if ( wc == 1 )
	{
		if ( pEcMbSlave->m_cMbxCmds != INITCMD_INACTIVE && pEcMbSlave->m_cMbxCmds < pEcMbSlave->m_nMbxCmds )
		{	// SOE init command
			PEcMailboxCmdDesc p = pEcMbSlave->m_ppMbxCmds[pEcMbSlave->m_cMbxCmds];
			if ( pEcMbSlave->m_oMbxCmds < SIZEOF_EcMailboxCmdDescSoeData(p) )
			{
				ETHERCAT_MBOX_HEADER		mbx={0};
				ETHERCAT_SOE_HEADER		soe=p->soe.head;
				unsigned short nSendData	= minS(GetSoEDataSizeEcMbSlave( pEcMbSlave,VG_OUT), (unsigned short)(SIZEOF_EcMailboxCmdDescSoeData(p) - pEcMbSlave->m_oMbxCmds));
				mbx.Type						= ETHERCAT_MBOX_TYPE_SOE;
				mbx.Length					= ETHERCAT_SOE_HEADER_LEN + nSendData;

				if ( nSendData < SIZEOF_EcMailboxCmdDescSoeData(p)-pEcMbSlave->m_oMbxCmds )
				{
					soe.InComplete		= 1;
					soe.FragmentsLeft	= (unsigned short)((SIZEOF_EcMailboxCmdDescSoeData(p)+GetSoEDataSizeEcMbSlave( pEcMbSlave,VG_OUT)-1-pEcMbSlave->m_oMbxCmds) / GetSoEDataSizeEcMbSlave( pEcMbSlave,VG_OUT)) - 1;
				}
				EcatMbxSendCmdReqEcMbSlave( pEcMbSlave, &p->soe.data[pEcMbSlave->m_oMbxCmds], &mbx, NULL, NULL, NULL, NULL, &soe);
				pEcMbSlave->m_oMbxCmds			+= nSendData;
			}
		}
		else if ( pEcMbSlave->m_pSoE->safe.bValid )
		{
			pEcMbSlave->m_pSoE->safe.retry = 0;
			if ( pEcMbSlave->m_pSoE->safe.soe.InComplete )
			{
				ETHERCAT_MBOX_HEADER	mbx	= {0};				
				unsigned char*		 pData	= (unsigned char*)ENDOF(pEcMbSlave->m_pSoE->safe.pCmd);				
				mbx.Type					= ETHERCAT_MBOX_TYPE_SOE;
				mbx.Length					= ETHERCAT_SOE_HEADER_LEN;
				
				pEcMbSlave->m_pSoE->safe.nOffset	+= GetSoEDataSizeEcMbSlave( pEcMbSlave,VG_OUT);					

				if ( pEcMbSlave->m_pSoE->safe.soe.FragmentsLeft > 1 )
				{
					pEcMbSlave->m_pSoE->safe.soe.FragmentsLeft--;
					mbx.Length							+= GetSoEDataSizeEcMbSlave( pEcMbSlave,VG_OUT);
				}
				else
				{
					pEcMbSlave->m_pSoE->safe.soe.IDN		= pEcMbSlave->m_pSoE->safe.nIDN;
					pEcMbSlave->m_pSoE->safe.soe.InComplete	= 0;
					mbx.Length		+= (unsigned short)(pEcMbSlave->m_pSoE->safe.pCmd->length - pEcMbSlave->m_pSoE->safe.nOffset);
				}
				pEcMbSlave->m_pSoE->safe.retry		= ECMAILBOX_DEFAULT_SOE_RETRY;
				Start(pEcMbSlave->m_pSoE->safe.timeout,ECMAILBOX_DEFAULT_SOE_TMOUT);

				EcatMbxSendCmdReqEcMbSlave( pEcMbSlave, pData+pEcMbSlave->m_pSoE->safe.nOffset, &mbx, NULL, NULL, NULL, NULL, &pEcMbSlave->m_pSoE->safe.soe);				
			}
		}

		// trigger mailbox polling -> server may want to response
		if ( pEcMbSlave->m_pSoE->safe.soe.InComplete == 0 && CycleMBoxPollingEcMbSlave( pEcMbSlave) )
			MBoxReadFromSlaveEcMbSlave( pEcMbSlave );
	}
}


 

///////////////////////////////////////////////////////////////////////////////
void	ProcessCoEResponseEcMbSlave(PEcMbSlave pEcMbSlave, PETHERCAT_MBOX_HEADER pMBox)
{
	if ( pEcMbSlave->m_pCoE == NULL )
		return;

	unsigned int msg = EC_LOG_MSGTYPE_LOG|EC_LOG_MSGTYPE_ERROR;
	PETHERCAT_CANOPEN_HEADER	pCan = (PETHERCAT_CANOPEN_HEADER)ENDOF(pMBox);
	if ( pCan->Type == ETHERCAT_CANOPEN_TYPE_SDORES )
	{
		PETHERCAT_SDO_HEADER			pSdo = (PETHERCAT_SDO_HEADER)ENDOF(pCan);
		if ( pEcMbSlave->m_cMbxCmds != INITCMD_INACTIVE && pEcMbSlave->m_cMbxCmds < pEcMbSlave->m_nMbxCmds )
		{				
			// CANopen init command
			switch ( pSdo->Ids.Scs )
			{
			case SDO_SCS_INITIATE_DOWNLOAD:
			case SDO_SCS_DOWNLOAD_SEGMENT:
				{	// succeeded
					PEcMailboxCmdDesc p = pEcMbSlave->m_ppMbxCmds[pEcMbSlave->m_cMbxCmds];
					if ( p->coe.sdo.Data-pEcMbSlave->m_oMbxCmds == 0 )
					{	// transfer complete -> next init cmd
						StartInitCmdsEcMbSlave( pEcMbSlave, pEcMbSlave->m_actTransition);
					}
					else
					{	// send next segment
						// nMaxData fits in mailbox
						unsigned short nMaxData	= pEcMbSlave->m_mbxOLen[pEcMbSlave->m_mbxIdx] - ETHERCAT_MIN_SDO_MBOX_LEN + SDO_DOWNLOAD_SEGMENT_MAX_DATA;
						unsigned short nSendData	= maxS(nMaxData, (unsigned short)(p->coe.sdo.Data - pEcMbSlave->m_oMbxCmds));
						Start(pEcMbSlave->m_tMbxCmds, p->timeout);

						ETHERCAT_MBOX_HEADER		mbx ={0};
						ETHERCAT_CANOPEN_HEADER can = {0};
						ETHERCAT_SDO_HEADER		sdo = {0};
						mbx.Length	= ETHERCAT_CANOPEN_HEADER_LEN + ETHERCAT_SDO_HEADER_LEN;
						mbx.Type		= ETHERCAT_MBOX_TYPE_CANOPEN;
						can.Type		= ETHERCAT_CANOPEN_TYPE_SDOREQ;
						sdo.Dsq.Ccs	= SDO_CCS_DOWNLOAD_SEGMENT;

						if ( pSdo->Ids.Scs == SDO_SCS_INITIATE_DOWNLOAD )	
							sdo.Dsq.Toggle		= 0;	// last cmd was an initiate
						else								
							sdo.Dsq.Toggle		= !pSdo->Dss.Toggle;

						if ( nSendData <= SDO_DOWNLOAD_SEGMENT_MAX_DATA )
						{	// data fits in 7 byte data area of sdo
							sdo.Dsq.Size		= SDO_DOWNLOAD_SEGMENT_MAX_DATA - nSendData;
						}
						else
						{	// fill th complete mailbox area with data
							sdo.Dsq.Size		= 0;
							mbx.Length			+= nSendData - SDO_DOWNLOAD_SEGMENT_MAX_DATA;
						}
						sdo.Dsq.LastSeg	= nSendData <= nMaxData;

						EcatMbxSendCmdReqEcMbSlave( pEcMbSlave, &p->coe.data[pEcMbSlave->m_oMbxCmds], &mbx, NULL, &can, &sdo);
						pEcMbSlave->m_oMbxCmds		+= nSendData;
					}
				}
				break;
			}
		}
		else if ( pEcMbSlave->m_pCoE->safe.bValid )
		{	
			// queued CANopen Mailbox command
			switch ( pSdo->Ids.Scs )
			{
			case SDO_SCS_INITIATE_UPLOAD:
				// response of an initiate upload
				if ( pSdo->Ius.Expedited && pSdo->Ius.SizeInd )
				{							
					// expidited transfer with size indicator					
					if ( pEcMbSlave->m_pCoE->safe.pCmd->length >= sizeof(unsigned long) - pSdo->Ius.Size )
					{	
						MailboxUploadResEcMbSlave( pEcMbSlave, pEcMbSlave->m_pCoE->safe.pCmd, ECERR_NOERR, sizeof(unsigned long) - pSdo->Ius.Size, &pSdo->Data);
						ClearCoESafeEcMbSlave( pEcMbSlave);
					}
					else
					{	// size mismatch (slave returned more data than Mailbox client has expected)
						StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDSIZE);
					}
				}
				else if ( pSdo->Ius.Expedited && !pSdo->Ius.SizeInd )
				{	// expidited transfer without size indicator					
					unsigned long length = min(pEcMbSlave->m_pCoE->safe.pCmd->length, sizeof(unsigned long));
					
					MailboxUploadResEcMbSlave( pEcMbSlave, pEcMbSlave->m_pCoE->safe.pCmd, ECERR_NOERR, length, &pSdo->Data);
					ClearCoESafeEcMbSlave( pEcMbSlave);
				}
				else if ( !pSdo->Ius.Expedited && pSdo->Ius.SizeInd )
				{	// normal transfer
					unsigned long nData = pMBox->Length - ETHERCAT_CANOPEN_HEADER_LEN - ETHERCAT_SDO_HEADER_LEN;					
					if ( nData == pSdo->Data )
					{	// initiate upload response contains all data (behind the 8 byte sdo header)
						if ( pEcMbSlave->m_pCoE->safe.pCmd->length >= nData )
						{	// size of Mailbox response is OK
							MailboxUploadResEcMbSlave( pEcMbSlave, pEcMbSlave->m_pCoE->safe.pCmd, ECERR_NOERR, nData, ENDOF(pSdo));
							ClearCoESafeEcMbSlave( pEcMbSlave);
						}
						else
						{	// size mismatch (slave returned more data than Mailbox client has expected)
							StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDSIZE);
						}
					}
					else if ( nData < pSdo->Data )
					{	// more data follows -> alloc Mailbox response						
						if ( pEcMbSlave->m_pCoE->safe.pCmd->length >= pSdo->Data )
						{	// size of Mailbox response is OK
							pEcMbSlave->m_pCoE->safe.pRetData = (unsigned char*)MailboxAlloc( pSdo->Data );
							if ( pEcMbSlave->m_pCoE->safe.pRetData )		
							{	// save returned data (if any) and request next segment
								pEcMbSlave->m_pCoE->safe.nRetData	= pSdo->Data;
								pEcMbSlave->m_pCoE->safe.nOffset		= 0;
								if ( nData > 0 )
								{
									pEcMbSlave->m_pCoE->safe.nOffset	= nData;
									memcpy(pEcMbSlave->m_pCoE->safe.pRetData, ENDOF(pSdo), nData);
								}
								// request next segment
								ETHERCAT_MBOX_HEADER		mbx={0};
								ETHERCAT_CANOPEN_HEADER	can={0};
								mbx.Type					= ETHERCAT_MBOX_TYPE_CANOPEN;
								mbx.Length				= ETHERCAT_CANOPEN_HEADER_LEN + ETHERCAT_SDO_HEADER_LEN;
								can.Type					= ETHERCAT_CANOPEN_TYPE_SDOREQ;
								pEcMbSlave->m_pCoE->safe.retry	= ECMAILBOX_DEFAULT_COE_RETRY;
								Start(pEcMbSlave->m_pCoE->safe.timeout, ECMAILBOX_DEFAULT_COE_TMOUT);

								memset(&pEcMbSlave->m_pCoE->safe.sdo, 0, sizeof(pEcMbSlave->m_pCoE->safe.sdo));
								pEcMbSlave->m_pCoE->safe.sdo.Usq.Ccs		= SDO_CCS_UPLOAD_SEGMENT;
								pEcMbSlave->m_pCoE->safe.sdo.Usq.Toggle	= 0;

								EcatMbxSendCmdReqEcMbSlave( pEcMbSlave, NULL, &mbx, NULL, &can, &pEcMbSlave->m_pCoE->safe.sdo);
							}
							else
							{	// memory allocation error -> send ABORT to the slave
								StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_NOMEMORY, SDO_ABORTCODE_MEMORY);
							}
						}
						else
						{	// size mismatch (slave returned more data than Mailbox client has expected)
							StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDSIZE, SDO_ABORTCODE_DATA_SIZE1);
						}
					}
					else
					{	// size mismatch (more data in mailbox than specified in sdo header)
						StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDSIZE, SDO_ABORTCODE_DATA_SIZE);
					}
				}
				else
				{	// invalid bits in sdo header
					StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDDATA, SDO_ABORTCODE_GENERAL);
				}
				break;
			case SDO_SCS_UPLOAD_SEGMENT:
				if ( pEcMbSlave->m_pCoE->safe.pRetData )
				{	// memory of return cmd must be pre allocated!
					if ( pSdo->Uss.Toggle == pEcMbSlave->m_pCoE->safe.sdo.Usq.Toggle )
					{	// toggle of segment is OK
						unsigned long nData = pMBox->Length - ETHERCAT_CANOPEN_HEADER_LEN - ETHERCAT_SDO_HEADER_LEN + SDO_DOWNLOAD_SEGMENT_MAX_DATA;
						
						if ( nData == SDO_DOWNLOAD_SEGMENT_MAX_DATA )	// if last segment size == 7 bytes
							nData -= pSdo->Uss.Size;							// consider size indicator
						if ( pSdo->Uss.LastSeg )
						{	// last segment
							if ( pEcMbSlave->m_pCoE->safe.nOffset + nData == pEcMbSlave->m_pCoE->safe.nRetData )
							{	// sumerized size is OK
								// save data
								memcpy(&pEcMbSlave->m_pCoE->safe.pRetData[pEcMbSlave->m_pCoE->safe.nOffset], &pSdo->Index, nData);
								// return to Mailbox client
								StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_NOERR, 0, pEcMbSlave->m_pCoE->safe.nRetData, pEcMbSlave->m_pCoE->safe.pRetData);
							}
							else
							{	// size mismatch
								StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDSIZE);
							}
						}
						else
						{	// a segment in the middle
							if ( pEcMbSlave->m_pCoE->safe.nOffset + nData < pEcMbSlave->m_pCoE->safe.nRetData )
							{	// sumerized data fits in pre allocated buffer -> copy data to buffer
								memcpy(&pEcMbSlave->m_pCoE->safe.pRetData[pEcMbSlave->m_pCoE->safe.nOffset], &pSdo->Index, nData);
								pEcMbSlave->m_pCoE->safe.nOffset		+= nData;

								// request new segment
								ETHERCAT_MBOX_HEADER		mbx={0};
								ETHERCAT_CANOPEN_HEADER	can={0};
								mbx.Type					= ETHERCAT_MBOX_TYPE_CANOPEN;
								mbx.Length				= ETHERCAT_CANOPEN_HEADER_LEN + ETHERCAT_SDO_HEADER_LEN;
								can.Type					= ETHERCAT_CANOPEN_TYPE_SDOREQ;
								pEcMbSlave->m_pCoE->safe.retry	= ECMAILBOX_DEFAULT_COE_RETRY;
								Start(pEcMbSlave->m_pCoE->safe.timeout, ECMAILBOX_DEFAULT_COE_TMOUT);

								memset(&pEcMbSlave->m_pCoE->safe.sdo, 0, sizeof(pEcMbSlave->m_pCoE->safe.sdo));
								pEcMbSlave->m_pCoE->safe.sdo.Usq.Ccs		= SDO_CCS_UPLOAD_SEGMENT;
								pEcMbSlave->m_pCoE->safe.sdo.Usq.Toggle	= !pSdo->Uss.Toggle;

								EcatMbxSendCmdReqEcMbSlave( pEcMbSlave, NULL, &mbx, NULL, &can, &pEcMbSlave->m_pCoE->safe.sdo);
							}
							else
							{	// buffer to small
								StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDSIZE, SDO_ABORTCODE_DATA_SIZE1);
							}
						}
					}
					else
					{	// toggle bit error
						StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDCONTEXT, SDO_ABORTCODE_TOGGLE);
					}
				}
				else
				{	// invalid state of sdoSafe (should never occur)
					StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDDATA);
				}
				break;
			case SDO_SCS_INITIATE_DOWNLOAD:
			case SDO_SCS_DOWNLOAD_SEGMENT:
				{
					if ( pEcMbSlave->m_pCoE->safe.nOffset < pEcMbSlave->m_pCoE->safe.pCmd->length )
					{	// more data to send
						unsigned short nMaxData		= pEcMbSlave->m_mbxOLen[pEcMbSlave->m_mbxIdx] - ETHERCAT_MIN_SDO_MBOX_LEN + SDO_DOWNLOAD_SEGMENT_MAX_DATA;
						unsigned short nSendData		= minS(nMaxData, (unsigned short)(pEcMbSlave->m_pCoE->safe.pCmd->length - pEcMbSlave->m_pCoE->safe.nOffset));
						ETHERCAT_MBOX_HEADER		mbx={0};
						ETHERCAT_CANOPEN_HEADER	can={0};
						mbx.Type					= ETHERCAT_MBOX_TYPE_CANOPEN;
						mbx.Length				= ETHERCAT_CANOPEN_HEADER_LEN + ETHERCAT_SDO_HEADER_LEN;
						can.Type					= ETHERCAT_CANOPEN_TYPE_SDOREQ;
						pEcMbSlave->m_pCoE->safe.retry	= ECMAILBOX_DEFAULT_COE_RETRY;
						Start(pEcMbSlave->m_pCoE->safe.timeout, ECMAILBOX_DEFAULT_COE_TMOUT);
						unsigned char* pData = &((unsigned char*)ENDOF(pEcMbSlave->m_pCoE->safe.pCmd))[pEcMbSlave->m_pCoE->safe.nOffset];

						memset(&pEcMbSlave->m_pCoE->safe.sdo, 0, sizeof(pEcMbSlave->m_pCoE->safe.sdo));

						pEcMbSlave->m_pCoE->safe.sdo.Dsq.Ccs		= SDO_CCS_DOWNLOAD_SEGMENT;
						if ( pSdo->Ids.Scs == SDO_SCS_INITIATE_DOWNLOAD )
							pEcMbSlave->m_pCoE->safe.sdo.Dsq.Toggle	= 0;
						else
							pEcMbSlave->m_pCoE->safe.sdo.Dsq.Toggle	= !pEcMbSlave->m_pCoE->safe.sdo.Dsq.Toggle;

						if ( nSendData < SDO_DOWNLOAD_SEGMENT_MAX_DATA )
						{
							pEcMbSlave->m_pCoE->safe.sdo.Dsq.Size		= SDO_DOWNLOAD_SEGMENT_MAX_DATA - nSendData;
							memcpy(&pEcMbSlave->m_pCoE->safe.sdo.Index, pData, nSendData);
						}
						else
						{
							pEcMbSlave->m_pCoE->safe.sdo.Dsq.Size		= 0;
							mbx.Length							+= nSendData - SDO_DOWNLOAD_SEGMENT_MAX_DATA;
							memcpy(&pEcMbSlave->m_pCoE->safe.sdo.Index, pData, SDO_DOWNLOAD_SEGMENT_MAX_DATA);
						}
						pEcMbSlave->m_pCoE->safe.sdo.Dsq.LastSeg	= nSendData <= nMaxData;

						if ( nSendData > SDO_DOWNLOAD_SEGMENT_MAX_DATA )
							EcatMbxSendCmdReqEcMbSlave( pEcMbSlave, &pData[SDO_DOWNLOAD_SEGMENT_MAX_DATA], &mbx, NULL, &can, &pEcMbSlave->m_pCoE->safe.sdo);
						else
							EcatMbxSendCmdReqEcMbSlave( pEcMbSlave,NULL, &mbx, NULL, &can, &pEcMbSlave->m_pCoE->safe.sdo);

						pEcMbSlave->m_pCoE->safe.nOffset	+= nSendData;
					}
					else
					{	// all data downloaded
						StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_NOERR);
					}
				}
				break;
			}
		}
	}
	else if ( pCan->Type == ETHERCAT_CANOPEN_TYPE_SDOREQ )
	{
		PETHERCAT_SDO_HEADER			pSdo = (PETHERCAT_SDO_HEADER)ENDOF(pCan);		
		if ( pEcMbSlave->m_cMbxCmds != INITCMD_INACTIVE )
		{							
			// CANopen init command
			switch ( pSdo->Idq.Ccs )
			{
			case SDO_CCS_ABORT_TRANSFER:
				{	// failed
					PEcMailboxCmdDesc p = pEcMbSlave->m_ppMbxCmds[pEcMbSlave->m_cMbxCmds];
					StartInitCmdsEcMbSlave( pEcMbSlave, pEcMbSlave->m_actTransition);
				}
				break;
			}
		}
		else if ( pEcMbSlave->m_pCoE->safe.bValid )
		{	
			// queued CANopen Mailbox command
			switch ( pSdo->Idq.Ccs )
			{
			case SDO_CCS_ABORT_TRANSFER:
				{
					unsigned long result=0;
					switch ( pSdo->Data )
					{
					case SDO_ABORTCODE_INDEX:
						result	= ECERR_DEVICE_INVALIDINDEX;
						break;
					case SDO_ABORTCODE_OFFSET:
						result	= ECERR_DEVICE_INVALIDOFFSET;
						break;
					case SDO_ABORTCODE_DATA_RANGE:
					case SDO_ABORTCODE_DATA_RANGE1:
					case SDO_ABORTCODE_DATA_RANGE2:
						result	= ECERR_DEVICE_INVALIDDATA;
						break;
					case SDO_ABORTCODE_READONLY:
					case SDO_ABORTCODE_WRITEONLY:
						result	= ECERR_DEVICE_INVALIDACCESS;
						break;
					case SDO_ABORTCODE_TIMEOUT:
						result	= ECERR_DEVICE_TIMEOUT;
						break;
					case SDO_ABORTCODE_TRANSFER2:
						result	= ECERR_DEVICE_INVALIDSTATE;
						break;
					default:
						result	= ECERR_DEVICE_SYNTAX;
					}
					StopCoESafeRequestEcMbSlave( pEcMbSlave, result);
				}
				break;
			}
		}
	}
	else if ( pCan->Type == ETHERCAT_CANOPEN_TYPE_SDOINFO )
	{
		if ( pEcMbSlave->m_pCoE->safe.bValid ) 
		{
			PETHERCAT_SDO_INFO_HEADER	pInfo = (PETHERCAT_SDO_INFO_HEADER)ENDOF(pCan);
			unsigned long	nData;
	 		nData	= pMBox->Length - ETHERCAT_CANOPEN_HEADER_LEN - offsetof(ETHERCAT_SDO_INFO_HEADER, Data);			
			if ( pInfo->InComplete == 0 )
			{
				switch ( pInfo->OpCode )
				{
				case ECAT_COE_INFO_OPCODE_LIST_S:
				case ECAT_COE_INFO_OPCODE_OBJ_S:
				case ECAT_COE_INFO_OPCODE_ENTRY_S:
					if ( pEcMbSlave->m_pCoE->safe.pRetData )
					{
						if ( nData + pEcMbSlave->m_pCoE->safe.nOffset <= pEcMbSlave->m_pCoE->safe.nRetData )
						{
							memcpy(&pEcMbSlave->m_pCoE->safe.pRetData[pEcMbSlave->m_pCoE->safe.nOffset], &pInfo->Data, nData);
							pEcMbSlave->m_pCoE->safe.nOffset += nData;
							if ( pEcMbSlave->m_pCoE->safe.nOffset <= pEcMbSlave->m_pCoE->safe.pCmd->length )
								StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_NOERR, 0, pEcMbSlave->m_pCoE->safe.nOffset, pEcMbSlave->m_pCoE->safe.pRetData);
							else
								StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDSIZE);
						}
						else
							StopCoESafeRequestEcMbSlave( pEcMbSlave,ECERR_DEVICE_INVALIDSIZE);
					}
					else
					{
						if ( nData <= pEcMbSlave->m_pCoE->safe.pCmd->length )
							StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_NOERR, 0, nData, &pInfo->Data);
						else
							StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDSIZE);
					}
					break;
				case ECAT_COE_INFO_OPCODE_ERROR_S:
					StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDDATA);
					break;
				default:
					StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDPARM);
				}
			}
			else
			{
				if ( pEcMbSlave->m_pCoE->safe.pRetData == NULL )
				{
					pEcMbSlave->m_pCoE->safe.nRetData	= nData * (pInfo->FragmentsLeft+1);
					pEcMbSlave->m_pCoE->safe.pRetData	= (unsigned char*)MailboxAlloc(pEcMbSlave->m_pCoE->safe.nRetData);
					pEcMbSlave->m_pCoE->safe.nOffset	= 0;
				}
				if ( pEcMbSlave->m_pCoE->safe.pRetData )
				{
					if ( nData + pEcMbSlave->m_pCoE->safe.nOffset <= pEcMbSlave->m_pCoE->safe.nRetData )
					{
						memcpy(&pEcMbSlave->m_pCoE->safe.pRetData[pEcMbSlave->m_pCoE->safe.nOffset], &pInfo->Data, nData);
						pEcMbSlave->m_pCoE->safe.nOffset += nData;

						// trigger mailbox polling -> server may want to sent more data
						if ( CycleMBoxPollingEcMbSlave( pEcMbSlave) )
							MBoxReadFromSlaveEcMbSlave( pEcMbSlave);
					}
					else
						StopCoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDSIZE);
				}
				else
					StopCoESafeRequestEcMbSlave( pEcMbSlave,ECERR_DEVICE_NOMEMORY);
			}
		}
	}
	else if ( pCan->Type == ETHERCAT_CANOPEN_TYPE_EMERGENCY ) 
	{
		PETHERCAT_EMERGENCY_HEADER	pEmgc = (PETHERCAT_EMERGENCY_HEADER)ENDOF(pCan);
	}
}
 

///////////////////////////////////////////////////////////////////////////////
void ProcessFoEResponseEcMbSlave( PEcMbSlave pEcMbSlave, PETHERCAT_MBOX_HEADER pMBox)
{	
	if ( pEcMbSlave->m_pFoE == NULL )
		return;

	unsigned long msg = EC_LOG_MSGTYPE_LOG|EC_LOG_MSGTYPE_ERROR;
	if ( pEcMbSlave->m_pFoE->safe.bValid )
	{
		PETHERCAT_FOE_HEADER	pFoE = (PETHERCAT_FOE_HEADER)ENDOF(pMBox);
		PEcMailboxCmdRW		pCmd = pEcMbSlave->m_pFoE->safe.pCmd;
		if ( pEcMbSlave->m_pFoE->safe.bRead )
		{
			switch (pFoE->OpCode)
			{
			case ECAT_FOE_OPCODE_DATA:
				if ( pEcMbSlave->m_pFoE->safe.nPacketNo == pFoE->PacketNo || pEcMbSlave->m_pFoE->safe.nPacketNo == pFoE->PacketNo+1 )
				{
					if ( pEcMbSlave->m_pFoE->safe.nPacketNo == pFoE->PacketNo )
					{
						unsigned long nData = pMBox->Length - ETHERCAT_FOE_HEADER_LEN;
						pEcMbSlave->m_pFoE->safe.bLastRead = nData < GetFoEDataSizeEcMbSlave( pEcMbSlave, VG_IN);	
						if ( pEcMbSlave->m_pFoE->safe.pRet )
						{
							PEcMailboxCmd	pCmdRes = pEcMbSlave->m_pFoE->safe.pRet;
							unsigned char* pByte = (unsigned char*)ENDOF(pCmdRes);

							unsigned long nFree = pCmd->length - pEcMbSlave->m_pFoE->safe.nOffset;
							unsigned long nAct	= min(nData, nFree);
							memcpy(&pByte[pEcMbSlave->m_pFoE->safe.nOffset], ENDOF(pFoE), nAct);
							pEcMbSlave->m_pFoE->safe.nOffset += nAct;

							nData -= nAct;
							nFree -= nAct;

						
							pCmdRes->length	+= nAct;

							if ( nData > 0 )
							{
								pEcMbSlave->m_pFoE->safe.nData = (unsigned short)nData;
								memcpy(pEcMbSlave->m_pFoE->safe.pData, ENDOF(pFoE), pEcMbSlave->m_pFoE->safe.nData);
							}
							if ( nFree == 0 || pEcMbSlave->m_pFoE->safe.bLastRead )
							{
								pEcMbSlave->m_pFoE->safe.pRet->result = ECERR_NOERR;
								MailboxResponseEcMaster((PEcMaster)pEcMbSlave->m_pEcSlave->m_pMaster, pEcMbSlave->m_pFoE->safe.pRet);
								pEcMbSlave->m_pFoE->safe.pRet = NULL;
								if ( pEcMbSlave->m_pFoE->safe.pCmd )
								{
									MailboxFree((PEcMailboxCmd)pEcMbSlave->m_pFoE->safe.pCmd);
									pEcMbSlave->m_pFoE->safe.pCmd		= NULL;
								}								
							}
						}
						else
						{	// temporary save data
							pEcMbSlave->m_pFoE->safe.nData = (unsigned short)nData;
							memcpy(pEcMbSlave->m_pFoE->safe.pData, ENDOF(pFoE), pEcMbSlave->m_pFoE->safe.nData);

							if ( pEcMbSlave->m_pFoE->safe.pCmd && pFoE->PacketNo == 1 )
							{	// comming from ADSIGRP_ECAT_FOE_FOPENREAD
								if (MailboxResEcMbSlave( pEcMbSlave, (PEcMailboxCmd)pEcMbSlave->m_pFoE->safe.pCmd, ECERR_NOERR, sizeof(pEcMbSlave->m_pFoE->fileHnd), &pEcMbSlave->m_pFoE->fileHnd) != ECERR_NOERR )
									StopFoESafeRequestEcMbSlave( pEcMbSlave,NOABORTSEND, ECAT_FOE_ERRCODE_DISKFULL);
								else
								{
									MailboxFree((PEcMailboxCmd)pEcMbSlave->m_pFoE->safe.pCmd);
									pEcMbSlave->m_pFoE->safe.pCmd		= NULL;									
								}
							}
						}
					}
					else
					{
						if ( pFoE->PacketNo == 0 )
						{	// invalid packetno
							StopFoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDARRAYIDX, ECAT_FOE_ERRCODE_PACKENO);
							break;
						}
					}
					if ( pEcMbSlave->m_pFoE->safe.nData == 0 )
					{
						ETHERCAT_MBOX_HEADER	mbx={0};
						ETHERCAT_FOE_HEADER	foe={0};
						mbx.Type				= ETHERCAT_MBOX_TYPE_FILEACCESS;
						mbx.Length				= ETHERCAT_FOE_HEADER_LEN;
						foe.OpCode				= ECAT_FOE_OPCODE_ACK;
						foe.PacketNo			= pFoE->PacketNo;
						pEcMbSlave->m_pFoE->safe.retry	= ECMAILBOX_DEFAULT_FOE_RETRY;
						Start(pEcMbSlave->m_pFoE->safe.timeout, ECMAILBOX_DEFAULT_FOE_TMOUT);
						pEcMbSlave->m_pFoE->safe.nPacketNo++;

						EcatMbxSendCmdReqEcMbSlave( pEcMbSlave, NULL, &mbx, NULL, NULL, NULL, &foe);
					}
				}
				else
					StopFoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDARRAYIDX, ECAT_FOE_ERRCODE_PACKENO);
				break;
			case ECAT_FOE_OPCODE_ERR:
				{
					long nTxt		= pMBox->Length - ETHERCAT_FOE_HEADER_LEN;
					unsigned long result	= ECERR_DEVICE_ERROR;
					switch(pFoE->ErrorCode)
					{
					case ECAT_FOE_ERRCODE_NOTDEFINED:		result	= ECERR_DEVICE_ERROR;				break;
					case ECAT_FOE_ERRCODE_NOTFOUND:			result	= ECERR_DEVICE_NOTFOUND;			break;
					case ECAT_FOE_ERRCODE_ACCESS:			result	= ECERR_DEVICE_ACCESSDENIED;		break;
					case ECAT_FOE_ERRCODE_DISKFULL:			result	= ECERR_DEVICE_NOMEMORY;			break;
					case ECAT_FOE_ERRCODE_ILLEAGAL:			result	= ECERR_DEVICE_INVALIDCONTEXT;	break;
					case ECAT_FOE_ERRCODE_PACKENO:			result	= ECERR_DEVICE_INVALIDSTATE;		break;
					case ECAT_FOE_ERRCODE_EXISTS:			result	= ECERR_DEVICE_EXISTS;				break;
					case ECAT_FOE_ERRCODE_NOUSER:			result	= ECERR_DEVICE_ERROR;				break;
					case ECAT_FOE_ERRCODE_BOOTSTRAPONLY:	result	= ECERR_DEVICE_INVALIDSTATE;		break;
					case ECAT_FOE_ERRCODE_NOTINBOOTSTRAP:	result	= ECERR_DEVICE_INVALIDSTATE;		break;
					}
					if ( nTxt > 0 )
						StopFoESafeRequestEcMbSlave( pEcMbSlave, result, NOABORTSEND, nTxt, (PCHAR)ENDOF(pFoE));
					else
						StopFoESafeRequestEcMbSlave( pEcMbSlave,result, NOABORTSEND);
				}
				break;
			default:
				StopFoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDCONTEXT, ECAT_FOE_ERRCODE_ILLEAGAL);
			}
		}
		else
		{
			switch (pFoE->OpCode)
			{
			case ECAT_FOE_OPCODE_ACK:
				if ( pEcMbSlave->m_pFoE && pEcMbSlave->m_pFoE->safe.bDataPend )
				{	// comming from a data command
					if ( pFoE->PacketNo == pEcMbSlave->m_pFoE->safe.nPacketNo )
					{
						pEcMbSlave->m_pFoE->safe.bDataPend	= 0;
						pEcMbSlave->m_pFoE->safe.nOffset	+= GetFoEDataSizeEcMbSlave( pEcMbSlave, VG_OUT);
						pEcMbSlave->m_pFoE->safe.nPacketNo++;

						if ( pCmd->writeLength >= GetFoEDataSizeEcMbSlave( pEcMbSlave,VG_OUT)+pEcMbSlave->m_pFoE->safe.nOffset )
						{	// send next data packet
							unsigned char*							pByte = (unsigned char*)ENDOF(pCmd);
							ETHERCAT_MBOX_HEADER		mbx={0};
							ETHERCAT_FOE_HEADER		foe={0};
							mbx.Type						= ETHERCAT_MBOX_TYPE_FILEACCESS;
							mbx.Length					= (unsigned short)(ETHERCAT_FOE_HEADER_LEN + GetFoEDataSizeEcMbSlave( pEcMbSlave, VG_OUT));
							foe.OpCode					= ECAT_FOE_OPCODE_DATA;
							foe.PacketNo				= pEcMbSlave->m_pFoE->safe.nPacketNo;
							pEcMbSlave->m_pFoE->safe.bDataPend	= 1;
							pEcMbSlave->m_pFoE->safe.retry		= ECMAILBOX_DEFAULT_FOE_RETRY;
							Start(pEcMbSlave->m_pFoE->safe.timeout, ECMAILBOX_DEFAULT_FOE_TMOUT);

							EcatMbxSendCmdReqEcMbSlave( pEcMbSlave,&pByte[pEcMbSlave->m_pFoE->safe.nOffset], &mbx, NULL, NULL, NULL, &foe);
						}
						else
						{	// need more data or an ADSIOFFS_ECAT_FOE_FCLOSE or if closing last ACK
							MailboxResEcMbSlave( pEcMbSlave, (PEcMailboxCmd)pEcMbSlave->m_pFoE->safe.pCmd, ECERR_NOERR, 0, NULL);
							if ( pCmd->cmdId == MAILBOXCMD_ECAT_FOE_FCLOSE )
								ClearFoESafeEcMbSlave( pEcMbSlave);
							else if ( pCmd->writeLength == pEcMbSlave->m_pFoE->safe.nOffset )
							{	// all actual data sent -> free data and wait for new data
								MailboxFree((PEcMailboxCmd)pEcMbSlave->m_pFoE->safe.pCmd);
								pEcMbSlave->m_pFoE->safe.pCmd		= NULL;								
							}
						}
					}
					else
						StopFoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDDATA, ECAT_FOE_ERRCODE_PACKENO);
				}
				else 
				{	// coming from an open command -> response with file handle
					if ( pFoE->PacketNo == 0 )
					{
						pEcMbSlave->m_pFoE->safe.nPacketNo = 1;
						if ( MailboxResEcMbSlave( pEcMbSlave, (PEcMailboxCmd)pEcMbSlave->m_pFoE->safe.pCmd, ECERR_NOERR, sizeof(pEcMbSlave->m_pFoE->fileHnd), &pEcMbSlave->m_pFoE->fileHnd) != ECERR_NOERR )
							StopFoESafeRequestEcMbSlave( pEcMbSlave, NOABORTSEND, ECAT_FOE_ERRCODE_DISKFULL);
						else
						{
							MailboxFree((PEcMailboxCmd)pEcMbSlave->m_pFoE->safe.pCmd);
							pEcMbSlave->m_pFoE->safe.pCmd		= NULL;							
						}
					}
					else
						StopFoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDDATA, ECAT_FOE_ERRCODE_PACKENO);
				}
				break;
			case ECAT_FOE_OPCODE_BUSY:
				if ( pEcMbSlave->m_pFoE->safe.bDataPend )
				{	// wait a while and send data again
					long nTxt				= pMBox->Length - ETHERCAT_FOE_HEADER_LEN;
					pEcMbSlave->m_pFoE->safe.bBusy	= 1;
					Start(pEcMbSlave->m_pFoE->safe.busy, ECMAILBOX_DEFAULT_FOE_BUSY);
					Start(pEcMbSlave->m_pFoE->safe.timeout,ECMAILBOX_DEFAULT_FOE_TMOUT);

					if ( nTxt > 0 )
						strncpy(pEcMbSlave->m_pFoE->safe.busyInfo.Comment, (PCHAR)ENDOF(pFoE), sizeof(pEcMbSlave->m_pFoE->safe.busyInfo.Comment));
					else
						pEcMbSlave->m_pFoE->safe.busyInfo.Comment[0] = '\0';
					pEcMbSlave->m_pFoE->safe.busyInfo.Done		= pFoE->Done;
					pEcMbSlave->m_pFoE->safe.busyInfo.Entire	= pFoE->Entire;
				}
				else 
					StopFoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDCONTEXT, ECAT_FOE_ERRCODE_ILLEAGAL);
				break;
			case ECAT_FOE_OPCODE_ERR:
				{
					long nTxt		= pMBox->Length - ETHERCAT_FOE_HEADER_LEN;
					unsigned long result	= ECERR_DEVICE_ERROR;
					switch(pFoE->ErrorCode)
					{
					case ECAT_FOE_ERRCODE_NOTDEFINED:		result	= ECERR_DEVICE_ERROR;				break;
					case ECAT_FOE_ERRCODE_NOTFOUND:			result	= ECERR_DEVICE_NOTFOUND;			break;
					case ECAT_FOE_ERRCODE_ACCESS:			result	= ECERR_DEVICE_ACCESSDENIED;		break;
					case ECAT_FOE_ERRCODE_DISKFULL:			result	= ECERR_DEVICE_NOMEMORY;			break;
					case ECAT_FOE_ERRCODE_ILLEAGAL:			result	= ECERR_DEVICE_INVALIDCONTEXT;		break;
					case ECAT_FOE_ERRCODE_PACKENO:			result	= ECERR_DEVICE_INVALIDSTATE;		break;
					case ECAT_FOE_ERRCODE_EXISTS:			result	= ECERR_DEVICE_EXISTS;				break;
					case ECAT_FOE_ERRCODE_NOUSER:			result	= ECERR_DEVICE_ERROR;				break;
					case ECAT_FOE_ERRCODE_BOOTSTRAPONLY:	result	= ECERR_DEVICE_INVALIDSTATE;		break;
					case ECAT_FOE_ERRCODE_NOTINBOOTSTRAP:	result	= ECERR_DEVICE_INVALIDSTATE;		break;
					}
					if ( nTxt > 0 )
						StopFoESafeRequestEcMbSlave( pEcMbSlave, result, NOABORTSEND, nTxt, (PCHAR)ENDOF(pFoE));
					else
						StopFoESafeRequestEcMbSlave( pEcMbSlave, result, NOABORTSEND);
				}
				break;
			default:
				StopFoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDCONTEXT, ECAT_FOE_ERRCODE_ILLEAGAL);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void	ProcessSoEResponseEcMbSlave( PEcMbSlave pEcMbSlave, PETHERCAT_MBOX_HEADER pMBox)
{
	PETHERCAT_SOE_HEADER	pSoE = (PETHERCAT_SOE_HEADER)ENDOF(pMBox);
	if ( pEcMbSlave->m_pSoE == NULL )
		return;

	unsigned long msg = EC_LOG_MSGTYPE_LOG|EC_LOG_MSGTYPE_ERROR;
	if ( pEcMbSlave->m_cMbxCmds != INITCMD_INACTIVE && pEcMbSlave->m_cMbxCmds < pEcMbSlave->m_nMbxCmds )
	{	// SOE init command
		PEcMailboxCmdDesc p = pEcMbSlave->m_ppMbxCmds[pEcMbSlave->m_cMbxCmds];
		if ( SIZEOF_EcMailboxCmdDescSoeData(p)-pEcMbSlave->m_oMbxCmds == 0 )
		{	// transfer complete -> next init cmd
			StartInitCmdsEcMbSlave( pEcMbSlave, pEcMbSlave->m_actTransition);
		}
	}
	else 
	{
		if ( pMBox->Length < ETHERCAT_SOE_HEADER_LEN )
		{
			StopSoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDDATA);
		}
		else if ( pSoE->Error )
		{
			StopSoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_ERROR + ((*(USHORT UNALIGNED*)ENDOF(pSoE)) << 16));
		}
		else
		{
			switch ( pSoE->OpCode )
			{
			case ECAT_SOE_OPCODE_RRS:
				if ( pEcMbSlave->m_pSoE->safe.bValid && pEcMbSlave->m_pSoE->safe.soe.OpCode == ECAT_SOE_OPCODE_RRQ )
				{	// pending read request
					
					unsigned long				nData = pMBox->Length - ETHERCAT_SOE_HEADER_LEN;
					if ( pSoE->InComplete )
					{	// reading data are incomplete
						if ( pEcMbSlave->m_pSoE->safe.pRetData )
						{	// buffer already exists
							if ( pEcMbSlave->m_pSoE->safe.nOffset + nData <= pEcMbSlave->m_pSoE->safe.nRetData )
							{	// data fit in buffer -> copy to buffer
								memcpy(&pEcMbSlave->m_pSoE->safe.pRetData[pEcMbSlave->m_pSoE->safe.nOffset], ENDOF(pSoE), nData);
								pEcMbSlave->m_pSoE->safe.nOffset += (unsigned short)nData;
							}
							else
							{	// invalid buffer size
								StopSoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDSIZE);
							}
						}
						else
						{	// no buffer exists -> first fragment
							pEcMbSlave->m_pSoE->safe.nOffset		= 0;
							pEcMbSlave->m_pSoE->safe.nRetData	= (pSoE->FragmentsLeft+1) * nData;	// maximal buffer size
							pEcMbSlave->m_pSoE->safe.pRetData	= NULL;
							if ( pEcMbSlave->m_pSoE->safe.nRetData > 0 )	// must be > 0
								pEcMbSlave->m_pSoE->safe.pRetData	= (unsigned char*)MailboxAlloc(pEcMbSlave->m_pSoE->safe.nRetData);
							if ( pEcMbSlave->m_pSoE->safe.pRetData )
							{	// copy fragment
								memcpy(&pEcMbSlave->m_pSoE->safe.pRetData[pEcMbSlave->m_pSoE->safe.nOffset], ENDOF(pSoE), nData);
								pEcMbSlave->m_pSoE->safe.nOffset += (unsigned short)nData;
							}
							else
								StopSoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_NOMEMORY);
						}
					}
					else
					{	// complete data or last fragment
						if ( pEcMbSlave->m_pSoE->safe.pRetData )
						{	// last fragment
							if ( pEcMbSlave->m_pSoE->safe.nOffset + nData <= pEcMbSlave->m_pSoE->safe.nRetData )
							{	// data fit in buffer -> copy to buffer and stop request normally
								memcpy(&pEcMbSlave->m_pSoE->safe.pRetData[pEcMbSlave->m_pSoE->safe.nOffset], ENDOF(pSoE), nData);
								StopSoESafeRequestEcMbSlave( pEcMbSlave, ECERR_NOERR, pEcMbSlave->m_pSoE->safe.nOffset + nData, pEcMbSlave->m_pSoE->safe.pRetData);
							}
							else
								StopSoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDSIZE);
						}
						else
						{	// complete data, no buffer required, stop request normally
							StopSoESafeRequestEcMbSlave( pEcMbSlave, ECERR_NOERR, nData, ENDOF(pSoE));
						}
					}
				}
				else if ( pEcMbSlave->m_pSoE->safe.bValid )
					StopSoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDACCESS);
				break;
			case ECAT_SOE_OPCODE_WRS:
				if ( pEcMbSlave->m_pSoE->safe.bValid && pEcMbSlave->m_pSoE->safe.soe.OpCode == ECAT_SOE_OPCODE_WRQ )
				{
					PECMBSLAVE_SOE_CMD_INFO pInfo = pEcMbSlave->m_pSoE->pCmds->Lookup(pEcMbSlave->m_pSoE->safe.soe.IDN);					
					if ( (pEcMbSlave->m_pSoE->safe.pCmd->indexOffset&MAILBOXIOFFS_ECAT_SOE_COMMAND) != 0 )
					{
						if ( pInfo )
						{
							if ( pInfo->pCmd == NULL && pInfo->state == ECMBSLAVE_SOE_CMD_STATE_0_WRITTEN )
							{
								unsigned long result = pInfo->result;
								pEcMbSlave->m_pSoE->pCmds->Remove(pEcMbSlave->m_pSoE->safe.soe.IDN);
								StopSoESafeRequestEcMbSlave( pEcMbSlave, result);
							}
						}
						else
						{	// new cmd
							ECMBSLAVE_SOE_CMD_INFO info;

							memset(&info, 0, sizeof(info));
							info.state		= ECMBSLAVE_SOE_CMD_STATE_3_WRITTEN;
							Start(info.timeout, 100000);		 
							info.pCmd		= pEcMbSlave->m_pSoE->safe.pCmd;
							pEcMbSlave->m_pSoE->safe.pCmd	= NULL;

							pEcMbSlave->m_pSoE->pCmds->Add(pEcMbSlave->m_pSoE->safe.soe.IDN, info);

							ClearSoESafeEcMbSlave( pEcMbSlave);
						}
					}
					else if ( pInfo )
					{	// change of a running command
						unsigned short commandValue = *(unsigned short*)ENDOF(pEcMbSlave->m_pSoE->safe.pCmd);
						switch ( commandValue )
						{
						case 0:
							if ( pInfo->pCmd )
							{	// end running command
								MailboxResEcMbSlave( pEcMbSlave,  pInfo->pCmd, ECERR_DEVICE_ABORTED);
								MailboxFree( pInfo->pCmd);
								pInfo->pCmd = NULL;
							}
							pEcMbSlave->m_pSoE->pCmds->Remove(pEcMbSlave->m_pSoE->safe.soe.IDN);
							break;
						}
						StopSoESafeRequestEcMbSlave( pEcMbSlave, ECERR_NOERR);
					}
					else
						StopSoESafeRequestEcMbSlave( pEcMbSlave, ECERR_NOERR);
				}
				else if ( pEcMbSlave->m_pSoE->safe.bValid )
					StopSoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_INVALIDACCESS);
				break;
			case ECAT_SOE_OPCODE_NFC:
				if ( pSoE->DataState )
				{
					unsigned short dataState = *(USHORT UNALIGNED*)ENDOF(pSoE);
					PECMBSLAVE_SOE_CMD_INFO pInfo = pEcMbSlave->m_pSoE->pCmds->Lookup(pSoE->IDN);
					if ( pInfo && pInfo->pCmd && pInfo->state == ECMBSLAVE_SOE_CMD_STATE_3_WRITTEN )
					{
						switch ( dataState )
						{
						case 0x3:	
							pInfo->result = ECERR_NOERR;					
							break;
						case 0x1:	
						case 0x5:	
						case 0x7:	
							pInfo->result = ECERR_DEVICE_PENDING;
							break;
						case 0xf:	
						default:
							pInfo->result = ECERR_DEVICE_ERROR;		
							break;
						}
						if ( pInfo->result != ECERR_DEVICE_PENDING )
						{
							pInfo->state				= ECMBSLAVE_SOE_CMD_STATE_0_WRITTEN;							
							*(unsigned long*)ENDOF(pInfo->pCmd) = 0;
							pEcMbSlave->m_pSoE->pPend->Add(pInfo->pCmd);
							pInfo->pCmd = NULL;
						}
					}
				}
				break;
			case ECAT_SOE_OPCODE_EMGCY:
				{
					PETHERCAT_EMERGENCY_HEADER	pEmgc = (PETHERCAT_EMERGENCY_HEADER)ENDOF(pSoE);
				}
				break;
			default:
				StopSoESafeRequestEcMbSlave( pEcMbSlave, ECERR_DEVICE_SRVNOTSUPP);
			}
		}
	}
}
