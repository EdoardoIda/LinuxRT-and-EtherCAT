#include <SPI.h>         // needed for Ethernet library communication with the W5100 (Arduino ver>0018)

#include "AxesBrainEcInterface.h"
#include "Strutture.h"
#include "AxesBrainEcNpfDevice.h"
#include "AxesBrainEcDevice.h"

extern short                     iChi;
void	EcatCmdResEcMbSlave(PEcMbSlave pEcMbSlave, unsigned long result, unsigned long invokeId, PETYPE_EC_HEADER pHead);


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////               Slave             ///////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
unsigned short	GetTargetState(unsigned short transition)
{
	switch ( transition )
	{
	case ECAT_INITCMD_P_I:
	case ECAT_INITCMD_B_I:
	case ECAT_INITCMD_S_I:
	case ECAT_INITCMD_O_I:		return DEVICE_STATE_INIT;
	case ECAT_INITCMD_I_P:
	case ECAT_INITCMD_S_P:
	case ECAT_INITCMD_O_P:		return DEVICE_STATE_PREOP;
	case ECAT_INITCMD_P_S:
	case ECAT_INITCMD_O_S:		return DEVICE_STATE_SAFEOP;
	case ECAT_INITCMD_S_O:		return DEVICE_STATE_OP;
	case ECAT_INITCMD_I_B:		return DEVICE_STATE_BOOTSTRAP;
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
void	ExecDcTimingEcSlave(PEcSlave pEcSlave, EC_DC_TIMING_STATE dcState)
{
	
	EcMaster* pMaster = (EcMaster*)pEcSlave->m_pMaster;	
	EcSlave* pSlave = pEcSlave;
	switch (dcState)
	{
	case EC_DC_TIMING_INIT:
		memset(pEcSlave->m_nDcPortDiff, 0, sizeof(pEcSlave->m_nDcPortDiff));
		pEcSlave->m_nDcPortCnt			= 2;
		pEcSlave->m_nDcTimings			= 0;
		pEcSlave->m_regDlStatus			= 0;
		pEcSlave->m_bDcSupport			= 0;
		pEcSlave->m_bDc64Support			= 0;		
		EcatCmdReqEcMaster(pMaster, pSlave, ECSI_DC_READ_FEATURES, EC_CMD_TYPE_APRD, pEcSlave->m_autoIncAddr, ESC_REG_FEATURES, sizeof(unsigned short), NULL);
		pEcSlave->m_bDcTimingPending = 1;
		break;
	case EC_DC_TIMING_ONLINE_INIT:
		memset(pEcSlave->m_nDcPortDiff, 0, sizeof(pEcSlave->m_nDcPortDiff));
		pEcSlave->m_nDcTimings			= 0;
		break;
	case EC_DC_TIMING_READ_LINK_STATUS:
		EcatCmdReqEcMaster(pMaster, pSlave, ECSI_DC_READ_LINK_STATUS, EC_CMD_TYPE_APRD, pEcSlave->m_autoIncAddr, 
			ESC_REG_ESC_STATUS, sizeof(unsigned short), NULL);
		pEcSlave->m_bDcTimingPending = 1;
		break;
	case EC_DC_TIMING_READ_TIMING:
		if ( pEcSlave->m_bDcSupport )
		{
			//t.Trace(TraceInfo,"ExecDcTimingEcSlave>>>------------EC_DC_TIMING_READ_TIMING, '%s'\n" ,pEcSlave->m_szName);
			EcatCmdReqEcMaster(pMaster, pSlave, ECSI_DC_READ_TIMING, EC_CMD_TYPE_APRD, pEcSlave->m_autoIncAddr, ESC_REG_DC_SOF_LATCH_A, 
				ESC_REG_DC_SYSTIME_OFFS-ESC_REG_DC_SOF_LATCH_A, NULL);
			pEcSlave->m_bDcTimingPending = 1;
		}
		break;
	case EC_DC_TIMING_WRITE_SYSTIME_OFFS:
		if (pEcSlave-> m_bDcSupport )
		{
			EcatCmdReqEcMaster(pMaster, pSlave, ECSI_DC_WRITE_SYSTIME_OFFS, EC_CMD_TYPE_APWR, pEcSlave->m_autoIncAddr, 
				ESC_REG_DC_SYSTIME_OFFS, sizeof(pEcSlave->m_nDcSysTimeOffs920), &pEcSlave->m_nDcSysTimeOffs920);
			pEcSlave->m_bDcTimingPending = 1;
		}
		break;
	case EC_DC_TIMING_WRITE_CLOCK_DELAY:
		if ( pEcSlave->m_bDcSupport )
		{
			EcatCmdReqEcMaster(pMaster, pSlave, ECSI_DC_WRITE_CLOCK_DELAY, EC_CMD_TYPE_APWR, pEcSlave->m_autoIncAddr, 
				ESC_REG_DC_SYSTIME_DELAY, sizeof(pEcSlave->m_nDcRefClockDelay928), &pEcSlave->m_nDcRefClockDelay928 );
			pEcSlave->m_bDcTimingPending = 1;
		}
		break;
	case EC_DC_TIMING_WRITE_SPEED_START:
		if ( pEcSlave->m_bDcSupport )
		{
			unsigned short speed[2] = {0x1000, 0};
			EcatCmdReqEcMaster(pMaster, pSlave, ECSI_DC_WRITE_SPEED_START, EC_CMD_TYPE_APWR, pEcSlave->m_autoIncAddr, 
				ESC_REG_DC_SPEED_START, sizeof(speed), &speed);
			pEcSlave->m_bDcTimingPending = 1;
		}
		break;
	case EC_DC_TIMING_CHECK_BUILD_ARMW:
		EcatCmdReqEcMaster(pMaster, pSlave, ECSI_DC_CHECK_BUILD_ARMW, EC_CMD_TYPE_APRD, pEcSlave->m_autoIncAddr, 
			ESC_REG_ESC_TYPE, sizeof(unsigned long), NULL);
		break;
	case EC_DC_TIMING_CALC_TIMING:
		if ( pEcSlave->m_bDcSupport )
		{
			if ( pEcSlave->m_nDcTimings > 0 )
			{
				pEcSlave->m_nDcPortDiff[EC_DB] /= pEcSlave->m_nDcTimings;
				pEcSlave->m_nDcPortDiff[EC_BC] /= pEcSlave->m_nDcTimings;
				pEcSlave->m_nDcPortDiff[EC_AD] /= pEcSlave->m_nDcTimings;
				pEcSlave->m_nDcTimings = 1;

				if ( (pEcSlave->m_regDlStatus & ESC_STATUS_LOOP_D_MASK) == ESC_STATUS_LOOP_D_LINK && pEcSlave->m_nDcPortDiff[EC_AD] < MIN_ECSLAVE_DELAY )
				{
					pEcSlave->m_regDlStatus &= ~ESC_STATUS_LOOP_D_MASK;
					pEcSlave->m_regDlStatus |= ESC_STATUS_LOOP_D_CLOSE;
					pEcSlave->m_nDcPortCnt--;
				}
				if ( (pEcSlave->m_regDlStatus & ESC_STATUS_LOOP_B_MASK) == ESC_STATUS_LOOP_B_LINK && pEcSlave->m_nDcPortDiff[EC_DB] < MIN_ECSLAVE_DELAY )
				{
					pEcSlave->m_regDlStatus &= ~ESC_STATUS_LOOP_B_MASK;
					pEcSlave->m_regDlStatus |= ESC_STATUS_LOOP_B_CLOSE;
					pEcSlave->m_nDcPortCnt--;
				}
				if ( (pEcSlave->m_regDlStatus & ESC_STATUS_LOOP_C_MASK) == ESC_STATUS_LOOP_C_LINK && pEcSlave->m_nDcPortDiff[EC_BC] < MIN_ECSLAVE_DELAY )
				{
					pEcSlave->m_regDlStatus &= ~ESC_STATUS_LOOP_C_MASK;
					pEcSlave->m_regDlStatus |= ESC_STATUS_LOOP_C_CLOSE;
					pEcSlave->m_nDcPortCnt--;
				}
			}
		}
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
long	DcCalcDelayAAEcSlave(PEcSlave pEcSlave)
{
	if ( !pEcSlave->m_bDcSupport )
		return 0;

	long delay=0;
	if ( EC_PORT_C_INUSE(pEcSlave->m_regDlStatus) )
		delay = pEcSlave->m_nDcPortDelayR[EC_A] + pEcSlave->m_nDcExecDelay + pEcSlave->m_nDcPortDiff[EC_AD] + pEcSlave->m_nDcPortDiff[EC_DB] + pEcSlave->m_nDcPortDiff[EC_BC] + pEcSlave->m_nDcPortDelayT[EC_A];
	else if ( EC_PORT_B_INUSE(pEcSlave->m_regDlStatus) )
		delay = pEcSlave->m_nDcPortDelayR[EC_A] + pEcSlave->m_nDcExecDelay + pEcSlave->m_nDcPortDiff[EC_AD] + pEcSlave->m_nDcPortDiff[EC_DB] + pEcSlave->m_nDcPortDelayT[EC_A];
	else if ( EC_PORT_D_INUSE(pEcSlave->m_regDlStatus) )
		delay = pEcSlave->m_nDcPortDelayR[EC_A] + pEcSlave->m_nDcExecDelay + pEcSlave->m_nDcPortDiff[EC_AD] + pEcSlave->m_nDcPortDelayT[EC_A];
	else 
		delay = pEcSlave->m_nDcPortDelayR[EC_A] + pEcSlave->m_nDcExecDelay + pEcSlave->m_nDcPortDelayT[EC_A];
	return delay;
}

bool	StateMachineDcSlave(PEcSlave pEcSlave)
{
	return ( pEcSlave->m_bDcTimingPending == 0 );
}

///////////////////////////////////////////////////////////////////////////////
///\brief Request EtherCAT slave to change to the specified state.
void	RequestStateEcSlave(PEcSlave pEcSlave , unsigned short state)
{
	pEcSlave->m_reqState = state;
}

 
///////////////////////////////////////////////////////////////////////////////
///\brief Executes the state machine of the EtherCAT slave.
int	StateMachineEcSlave(PEcSlave pEcSlave, short iPino )
{

	if ( iPino == 0 )
	{

		PEcMbSlave pEcMbSlave;
		pEcMbSlave = (PEcMbSlave)pEcSlave->m_pEcMbSlave;
		if ( pEcMbSlave != NULL )
		{
			if (pEcSlave->m_bDcSupport)
			{
				return  (StateMachineEcMbSlave(pEcMbSlave) && StateMachineDcSlave(pEcSlave));
			}
			else
			{
				return  StateMachineEcMbSlave(pEcMbSlave);
			}
		}
	}


	if ( pEcSlave->m_currState != pEcSlave->m_reqState )
	{
		unsigned short	transition = 0;

		unsigned short	currState ;
		unsigned short	reqState ;
		currState = pEcSlave->m_currState;
		reqState = pEcSlave->m_reqState;

		switch ( pEcSlave->m_currState )
		{
		case DEVICE_STATE_INIT:			
			switch ( pEcSlave->m_reqState )
			{
			case DEVICE_STATE_PREOP:
			case DEVICE_STATE_SAFEOP:
			case DEVICE_STATE_OP:
				transition		= ECAT_INITCMD_I_P;
				break;
			case DEVICE_STATE_BOOTSTRAP:			
				transition		= ECAT_INITCMD_I_B;
			}
			break;
		case DEVICE_STATE_PREOP:
			switch ( pEcSlave->m_reqState )
			{
			case DEVICE_STATE_INIT:
			case DEVICE_STATE_BOOTSTRAP:	
				transition		= ECAT_INITCMD_P_I;
				break;
			case DEVICE_STATE_SAFEOP:
			case DEVICE_STATE_OP:
				transition		= ECAT_INITCMD_P_S;
				break;
			}
			break;
		case DEVICE_STATE_BOOTSTRAP:
			switch ( pEcSlave->m_reqState )
			{
			case DEVICE_STATE_INIT:
			case DEVICE_STATE_PREOP:
			case DEVICE_STATE_SAFEOP:
			case DEVICE_STATE_OP:
				transition		= ECAT_INITCMD_B_I;
				break;
			}		
		case DEVICE_STATE_SAFEOP:
			switch ( pEcSlave->m_reqState )
			{
			case DEVICE_STATE_INIT:
			case DEVICE_STATE_BOOTSTRAP: 
				transition		= ECAT_INITCMD_S_I;
				break;
			case DEVICE_STATE_PREOP:
				transition		= ECAT_INITCMD_S_P;
				break;
			case DEVICE_STATE_OP:
				transition		= ECAT_INITCMD_S_O;
				break;
			}
			break;
		case DEVICE_STATE_OP:
			switch ( pEcSlave->m_reqState )
			{
			case DEVICE_STATE_INIT:
			case DEVICE_STATE_BOOTSTRAP:  
				transition		= ECAT_INITCMD_O_I;
				break;
			case DEVICE_STATE_PREOP:
				transition		= ECAT_INITCMD_O_P;
				break;
			case DEVICE_STATE_SAFEOP:
				transition		= ECAT_INITCMD_O_S;
				break;
			}
			break;
		}		
		if ( transition && pEcSlave->m_cInitCmds == INITCMD_INACTIVE )
		{	//transition set and no init commands active -> send first init command defined for this transition
			StartInitCmdsEcSlave(pEcSlave, transition, 0);
		}
	}
	
	if (pEcSlave->m_bDcSupport)
	{
		return  ((pEcSlave->m_reqState == pEcSlave->m_currState) && StateMachineDcSlave(pEcSlave));
	}
	else
	{
		return pEcSlave->m_reqState == pEcSlave->m_currState;
	}
}

///////////////////////////////////////////////////////////////////////////////
///\brief Processes init commands that should be sent in this transition.
void StartInitCmdsEcSlave(PEcSlave pEcSlave, unsigned short transition, short iPino)
{		
	//this method is called once by CEcSlave::StateMachine to send the first
	//command defined for this transition and is called by CEcSlave:EcatCmdRes
	//for the following commands
	if ( iPino == 0 )
	{		

		PEcMbSlave pEcMbSlave;
		pEcMbSlave = (PEcMbSlave)pEcSlave->m_pEcMbSlave;
		if ( pEcMbSlave != NULL )
		{
			StartInitCmdsEcMbSlave(pEcMbSlave, transition);
			return;
		}
	}


	if ( pEcSlave->m_cInitCmds == INITCMD_INACTIVE )
		pEcSlave->m_cInitCmds = 0;
	else
		pEcSlave->m_cInitCmds++;

	//look for an init command that should be sent during this transition
	while ( pEcSlave->m_cInitCmds < pEcSlave->m_nInitCmds )
	{
		if ( pEcSlave->m_ppInitCmds[pEcSlave->m_cInitCmds]->transition & transition )
			//init command found -> send init command
			break;
		pEcSlave->m_cInitCmds++;
	}
	if ( pEcSlave->m_cInitCmds < pEcSlave->m_nInitCmds )
	{//init command found	
		PEcInitCmdDesc p	= pEcSlave->m_ppInitCmds[pEcSlave->m_cInitCmds];
		//set timeout for init command
		Start(pEcSlave->m_tInitCmds, p->timeout);
		pEcSlave->m_rInitCmds			= p->retries;
		//send init command to slave
		EcatCmdReqEcMaster( (PEcMaster)pEcSlave->m_pMaster, pEcSlave, transition, p->ecHead.cmd, p->ecHead.adp, p->ecHead.ado, 
			p->ecHead.len, EcInitCmdDescData(p));
	}
	else
	{
		StopInitCmdsEcSlave(pEcSlave);
		pEcSlave->m_currState = GetTargetState(transition);
	}
}

///////////////////////////////////////////////////////////////////////////////
///\ Stops the processing of init commands
void	StopInitCmdsEcSlave(PEcSlave pEcSlave)
{
	pEcSlave->m_cInitCmds = INITCMD_INACTIVE;
	PEcMbSlave pEcMbSlave;
	pEcMbSlave = (PEcMbSlave)pEcSlave->m_pEcMbSlave;
	if ( pEcMbSlave != NULL )
	{
		StopInitCmdsEcMbSlave(pEcMbSlave);
		return;
	}

}

///////////////////////////////////////////////////////////////////////////////
int	InitCmdsActiveEcSlave(PEcSlave pEcSlave)
{
	int iReturn;

	iReturn =  pEcSlave->m_cInitCmds != INITCMD_INACTIVE;

	PEcMbSlave pEcMbSlave;
	pEcMbSlave = (PEcMbSlave)pEcSlave->m_pEcMbSlave;
	if ( pEcMbSlave != NULL )
	{
		iReturn |= InitCmdsActiveEcMbSlave( pEcMbSlave );
	}
	return iReturn;
}


///////////////////////////////////////////////////////////////////////////////
///\brief Processes the response of an EtherCAT command.
void	EcatCmdResEcSlave(PEcSlave pEcSlave, unsigned long result, unsigned long invokeId, PETYPE_EC_HEADER pHead, short iPino)
{
	//if (pEcSlave->m_bDcRefClock && (invokeId >= ECSI_DC_READ_FEATURES && invokeId <= ECSI_DC_WRITE_IRQ_START))
	//{
		EcMaster* pMaster = (EcMaster *)pEcSlave->m_pMaster;
		
		switch ( invokeId )
		{
		case ECSI_DC_READ_FEATURES:
			pEcSlave->m_bDcSupport			= 0;
			pEcSlave->m_bDc64Support			= 0;
			pEcSlave->m_bDcTimingPending	= 0;
			if ( result == ECERR_NOERR )
			{
				unsigned short features;
				if ( ETYPE_EC_CMD_COUNT(pHead) == 1 && pHead->len == sizeof(features) )
				{				
					features = *((unsigned short*)(ENDOF(pHead)));
					if ( (features&ESC_FEATURE_DC_AVAILABLE) != 0 )	
						pEcSlave->m_bDcSupport = 1;
					if ( (features&ESC_FEATURE_DC64_AVAILABLE) != 0 )	
						pEcSlave->m_bDc64Support = 1;				
				}
			}
			break;
		case ECSI_DC_READ_LINK_STATUS:
			pEcSlave->m_bDcTimingPending = 0;
			if ( result == ECERR_NOERR )
			{
				if ( ETYPE_EC_CMD_COUNT(pHead) == 1 && pHead->len == sizeof(pEcSlave->m_regDlStatus) )
				{
					pEcSlave->m_regDlStatus	= *(USHORT UNALIGNED*)ENDOF(pHead);
					pEcSlave->m_nDcPortCnt	= 0;
					
					if ( EC_PORT_A_INUSE(pEcSlave->m_regDlStatus) )
					{
	// 					pEcSlave->m_nDcPortDelay[0] = 0;
						pEcSlave->m_nDcPortCnt++;
					}
					if ( EC_PORT_B_INUSE(pEcSlave->m_regDlStatus) )
					{
	//					pEcSlave->m_nDcPortDelay[1] = 0;
						pEcSlave->m_nDcPortCnt++;
					}
					if ( EC_PORT_C_INUSE(pEcSlave->m_regDlStatus) )
					{
	//					pEcSlave->m_nDcPortDelay[2] = 0;
						pEcSlave->m_nDcPortCnt++;
					}
					if ( EC_PORT_D_INUSE(pEcSlave->m_regDlStatus) )
					{
	//					pEcSlave->m_nDcPortDelay[3] = 0;
						pEcSlave->m_nDcPortCnt++;
					}
				}
			}
			else
			{
				pEcSlave->m_nDcPortCnt	= 2;
				pEcSlave->m_regDlStatus	= 0x1a01;
			}
			break;
		case ECSI_DC_READ_TIMING:
			//t.Trace(TraceInfo,"case ECSI_DC_READ_TIMING %s\n", pEcSlave->m_szName);

			pEcSlave->m_bDcTimingPending = 0;
			if ( result == ECERR_NOERR )
			{
			//t.Trace(TraceInfo,"case ECSI_DC_READ_TIMING No error %s\n", pEcSlave->m_szName);
				if ( ETYPE_EC_CMD_COUNT(pHead) == 1 && pHead->len == ESC_REG_DC_SYSTIME_OFFS-ESC_REG_DC_SOF_LATCH_A )
				{							
					ULONG UNALIGNED* pUlong = (PULONG)ENDOF(pHead);
					if ( pEcSlave->m_bDc64Support )
						pEcSlave->m_nDcSysTimeOffs920 = 0 - ((ULONGLONG UNALIGNED*)ENDOF(pHead))[3];
					else
						pEcSlave->m_nDcSysTimeOffs920 = 0 - pUlong[EC_A];


					if ( (!EC_PORT_A_INUSE(pEcSlave->m_regDlStatus) || pEcSlave->m_dcReceiveTime[0] != pUlong[0]) &&
						(!EC_PORT_B_INUSE(pEcSlave->m_regDlStatus) || pEcSlave->m_dcReceiveTime[1] != pUlong[1]) &&
						(!EC_PORT_C_INUSE(pEcSlave->m_regDlStatus) || pEcSlave->m_dcReceiveTime[2] != pUlong[2]) &&
						(!EC_PORT_D_INUSE(pEcSlave->m_regDlStatus) || pEcSlave->m_dcReceiveTime[3] != pUlong[3]) )
					{
						if ( EC_PORT_D_INUSE(pEcSlave->m_regDlStatus) )
						{
//t.Trace(TraceInfo,"%s : %x %x %x %x\n", pEcSlave->m_szName, pUlong[EC_A], pUlong[EC_B], pUlong[EC_C], pUlong[EC_D]);
							pEcSlave->m_nDcPortDiff[EC_AD] += pUlong[EC_D] - pUlong[EC_A];

							if ( EC_PORT_B_INUSE(pEcSlave->m_regDlStatus) )
							{
								pEcSlave->m_nDcPortDiff[EC_DB] += pUlong[EC_B] - pUlong[EC_D];

								if ( EC_PORT_C_INUSE(pEcSlave->m_regDlStatus) )
								{
									pEcSlave->m_nDcPortDiff[EC_BC] += pUlong[EC_C] - pUlong[EC_B];
								}
							}
							else if ( EC_PORT_C_INUSE(pEcSlave->m_regDlStatus) )
							{
								pEcSlave->m_nDcPortDiff[EC_DC] += pUlong[EC_C] - pUlong[EC_D];
							}
						}
						else if ( EC_PORT_B_INUSE(pEcSlave->m_regDlStatus) )
						{
							pEcSlave->m_nDcPortDiff[EC_AB] += pUlong[EC_B] - pUlong[EC_A];

							if ( EC_PORT_C_INUSE(pEcSlave->m_regDlStatus) )
							{
								pEcSlave->m_nDcPortDiff[EC_BC] += pUlong[EC_C] - pUlong[EC_B];
//t.Trace(TraceInfo,"%s : %x %x %x BA %d CB %d\n",pEcSlave->m_szName, pUlong[EC_A], pUlong[EC_B], pUlong[EC_C], pUlong[EC_B]-pUlong[EC_A], pUlong[EC_C]-pUlong[EC_B]);
							}
						}
						else if ( EC_PORT_C_INUSE(pEcSlave->m_regDlStatus) )
						{
							pEcSlave->m_nDcPortDiff[EC_AC] += pUlong[EC_C] - pUlong[EC_A];
						}
						memcpy(pEcSlave->m_dcReceiveTime, pUlong, sizeof(pEcSlave->m_dcReceiveTime));
						pEcSlave->m_nDcTimings++;
					}
				}
			}
			break;
		case ECSI_DC_WRITE_CLOCK_DELAY:
		case ECSI_DC_WRITE_SYSTIME_OFFS:
		case ECSI_DC_WRITE_SPEED_START:
		case ECSI_DC_WRITE_CYCLE_TIME:
	//	case ECSI_DC_WRITE_IRQ_START:
			pEcSlave->m_bDcTimingPending = 0;
			break;
		case ECSI_DC_CHECK_BUILD_ARMW:
			if ( result == ECERR_NOERR && ETYPE_EC_CMD_COUNT(pHead) == 1 && pHead->len == sizeof(unsigned short) )
			{
				unsigned long typeRefBuild = *(ULONG UNALIGNED*)ENDOF(pHead);
				//if ( LOBYTE(LOWORD(typeRefBuild)) <= 3 && HIWORD(typeRefBuild) < 3 )
				//{
					//pMaster->ReportLog(EC_LOG_MSGTYPE_WARN|EC_LOG_MSGTYPE_LOG, 
					//	"WARNING: '%s' (%d) is physical before the DC reference clock and may not support the ARMW command. This results in wrong DC timings", 
					//	GetName(), GetPhysicalAddress());
				//}
			}
			break;
		}
	//}
	//else
	//{	
		PEcMbSlave pEcMbSlave;
		if ( iPino == 0 )
		{

			pEcMbSlave = (PEcMbSlave)pEcSlave->m_pEcMbSlave;
			if ( pEcMbSlave != NULL )
			{
				EcatCmdResEcMbSlave((PEcMbSlave)pEcSlave->m_pEcMbSlave, result, invokeId, pHead);
				return;
			}
		}

		switch ( invokeId )
		{
		case ECAT_INITCMD_I_P:
		case ECAT_INITCMD_P_S:
		case ECAT_INITCMD_P_I:
		case ECAT_INITCMD_S_P:
		case ECAT_INITCMD_S_O:
		case ECAT_INITCMD_S_I:
		case ECAT_INITCMD_O_S:
		case ECAT_INITCMD_O_P:
		case ECAT_INITCMD_O_I:
		case ECAT_INITCMD_B_I:
		case ECAT_INITCMD_I_B:	
			if ( pEcSlave->m_cInitCmds < pEcSlave->m_nInitCmds )
			{	
				PEcInitCmdDesc pIC = pEcSlave->m_ppInitCmds[pEcSlave->m_cInitCmds];
				if ( result == ECERR_NOERR )
				{
					if ( pIC->cnt == 0xffff || ETYPE_EC_CMD_COUNT(pHead) == pIC->cnt )
					{	//valid working counter
						int bNext = 1;
						if ( pIC->validate && pHead->len == pIC->ecHead.len )
						{	// initcmd with validation
							unsigned char * pData = (unsigned char * )ENDOF(pHead);
							unsigned long nmData[2];
							if ( pIC->validateMask )
							{
								if ( pHead->len <= sizeof(nmData) )
									memcpy(nmData, pData, pHead->len);
								unsigned char *  pMask = (unsigned char * )EcInitCmdDescVMData(pIC);
								for ( int i=0; i < pHead->len; i++ )
									pData[i] &= pMask[i];
							}
							if ( memcmp(pData, EcInitCmdDescVData(pIC), pHead->len) != 0 )
							{	// validate failed -> retry
							  	iChi = 9;	
                                                        	if ( IsElapsed(pEcSlave->m_tInitCmds) )
								{
									pEcSlave->m_reqState	= pEcSlave->m_currState;
									StopInitCmdsEcSlave(pEcSlave);
								}
								else
								{
									EcatCmdReqEcMaster((PEcMaster)pEcSlave->m_pMaster,pEcSlave, invokeId, pIC->ecHead.cmd, pIC->ecHead.adp, pIC->ecHead.ado, 
										pIC->ecHead.len, EcInitCmdDescData(pIC));
								}
								bNext = 0;
							}
						}
						if ( bNext )
						{	//init command is valid -> process next init command	
							StartInitCmdsEcSlave(pEcSlave,(unsigned short)invokeId,0);
						}
					}
					else
					{	// invalid working counter -> retry					
						PEcInitCmdDesc p = pEcSlave->m_ppInitCmds[pEcSlave->m_cInitCmds];
						if ( pEcSlave->m_rInitCmds != 0 )
						{
							EcatCmdReqEcMaster((PEcMaster)pEcSlave->m_pMaster,pEcSlave, invokeId, pIC->ecHead.cmd, pIC->ecHead.adp, pIC->ecHead.ado, 
								pIC->ecHead.len, EcInitCmdDescData(pIC));
						}
						else
						{
							pEcSlave->m_reqState = pEcSlave->m_currState;
							StopInitCmdsEcSlave( pEcSlave );
						}
						pEcSlave->m_rInitCmds--;
					}
				}
				else
				{	//error result
					if ( pEcSlave->m_rInitCmds > 0 )
					{	// retry
						EcatCmdReqEcMaster((PEcMaster)pEcSlave->m_pMaster,pEcSlave, invokeId, pIC->ecHead.cmd, pIC->ecHead.adp, pIC->ecHead.ado, 
							pIC->ecHead.len, EcInitCmdDescData(pIC));
						pEcSlave->m_rInitCmds--;
					}
					else
					{
						pEcSlave->m_reqState = pEcSlave->m_currState;
						StopInitCmdsEcSlave( pEcSlave );
					}
				}
			}
			break;
		}
	//}
}





