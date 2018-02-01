#include <SPI.h>         // needed for Ethernet library communication with the W5100 (Arduino ver>0018)

#include "AxesBrainEcInterface.h"
#include "Strutture.h"
#include "AxesBrainEcNpfDevice.h"
#include "AxesBrainEcDevice.h"


extern EC_DECLARE_CRITSEC

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////             EcSync             ///////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

///\brief Processes a returning EtherCAT frame sent with SendEcFrame
///
///\param nResult result
///\param nInvokeId invokeId passed in SendEcFrame
///\param cbLength length of the EtherCAT Frame
///\param pFrame EtherCAT frame
void OnReturningEcFrameEcSyncClient(PEcSyncClient pEcSyncClient, unsigned long nResult, unsigned long nInvokeId, unsigned long cbLength, PETYPE_EC_HEADER pFrame)
{
	
		
	if( pEcSyncClient->m_syncDevReq.pFrame && pEcSyncClient->m_syncDevReq.nInvokeId == nInvokeId )
	{										
		pEcSyncClient->m_syncDevReq.nResult = nResult;			
		if( nResult == ECERR_NOERR )
		{
			if(pEcSyncClient->m_syncDevReq.cbLength == cbLength )
			{
				memcpy(pEcSyncClient->m_syncDevReq.pFrame, pFrame, cbLength);									
			}
			else
				pEcSyncClient->m_syncDevReq.nResult = ECERR_CLIENT_ERROR;					
		}			
		pEcSyncClient->m_syncDevReq.pFrame = NULL;
	}
		
}



///\brief Mailbox response to a previously sent mailbox cmd
///
///\param pCmd Mailbox command.
///\pMailbox Mailbox that sent the command	
void OnMailboxResponseEcSyncClient(PEcSyncClient pEcSyncClient, PEcMailboxCmd pCmd, PEcMailbox  pMailbox)
{	
		
	if( !pEcSyncClient->m_syncMbReq.bReceived && pEcSyncClient->m_syncMbReq.nInvokeId == pCmd->invokeId )
	{																			
		pEcSyncClient->m_syncMbReq.nResult = pCmd->result;
		if( pEcSyncClient->m_syncMbReq.nResult == ECERR_NOERR )
		{				
			if( pCmd->length > 0 )
			{
				if( pEcSyncClient->m_syncMbReq.pData )
				{
					if( pEcSyncClient->m_syncMbReq.cbLength >= pCmd->length )
					{
						pEcSyncClient->m_syncMbReq.cbLength = pCmd->length;
						memcpy(pEcSyncClient->m_syncMbReq.pData, ENDOF(pCmd), pCmd->length);										
					}
					else
						pEcSyncClient->m_syncMbReq.nResult = ECERR_CLIENT_ERROR;					
				}
				else
					pEcSyncClient->m_syncMbReq.nResult = ECERR_CLIENT_ERROR;					
			}			
		}				
		pEcSyncClient->m_syncMbReq.bReceived = 1;
	}
		
}

///\brief Sends an EtherCAT Frame
///
///\param nInvokeId number to identify the sent frame with
///\param cbLength length of the EtherCAT Frame
///\param pFrame EtherCAT frame	
long SendSyncEcFrameEcSyncClient(PEcSyncClient pEcSyncClient, unsigned long invokeId, unsigned long cbLength, PETYPE_EC_HEADER pSndFrame, PETYPE_EC_HEADER pRcvFrame)
{

	if( pSndFrame == NULL || pRcvFrame == NULL )
		return ECERR_CLIENT_INVALIDPARM;
	
	if( pEcSyncClient->m_pDevice == NULL )	
		return ECERR_CLIENT_ERROR;
	
	long nResult;
	{
//////////!!!!!!!!!!!!!!!///////// 		MSG msg;

		pEcSyncClient->m_syncDevReq.hEvent		= NULL;
		pEcSyncClient->m_syncDevReq.nInvokeId	= invokeId;			
		pEcSyncClient->m_syncDevReq.nResult		= ECERR_NOERR;
		pEcSyncClient->m_syncDevReq.cbLength	= cbLength;
		pEcSyncClient->m_syncDevReq.pFrame		= pRcvFrame;

		nResult = SendEcFrameEcDevice(pEcSyncClient->m_pDevice, invokeId, cbLength, pSndFrame);
		if( nResult == ECERR_NOERR )
		{
			if( pEcSyncClient->m_syncDevReq.pFrame == NULL )				
				nResult = pEcSyncClient->m_syncDevReq.nResult;
			
			memset(&pEcSyncClient->m_syncDevReq, 0, sizeof(EcSyncDevReq));
		}
	}
	return nResult;
}


///\brief Sends a mailbox command
///	
///\return Result of the call.		
long SyncSendMailboxCmdEcSyncClient(PEcSyncClient pEcSyncClient,PEcMailboxCmd pCmd, unsigned long cbRdLength, unsigned char *pRdData, unsigned long &cbRead)
{				
	long nResult;
	{
//////////!!!!!!!!!!!!!!!///////// 		MSG msg;						
		pEcSyncClient->m_syncMbReq.hEvent		= NULL;
		pEcSyncClient->m_syncMbReq.nInvokeId	= pCmd->invokeId;			
		pEcSyncClient->m_syncMbReq.nResult		= ECERR_NOERR;
		pEcSyncClient->m_syncMbReq.cbLength	= cbRdLength;
		pEcSyncClient->m_syncMbReq.pData		= pRdData;
		pEcSyncClient->m_syncMbReq.bReceived	= 0;

		nResult = SendMailboxCmd(pEcSyncClient->m_pMailbox, pCmd);
		if( nResult == ECERR_NOERR )
		{
		}
	}
	return nResult;
}	


//////////////////////////////////////////////////////////////////////////////////////////
long SyncUploadReqEcSyncClient(PEcSyncClient pEcSyncClient,unsigned long nInvokeId, unsigned short nSlave, unsigned long nCmdId, unsigned short nIndex, unsigned char nSubIndex, unsigned long cbRdLength, unsigned char *pRdData, unsigned long &cbRead)
{
	if( cbRdLength > 0 && pRdData == NULL)
		return ECERR_CLIENT_INVALIDPARM;

	if( pEcSyncClient->m_pMailbox == NULL )	
		return ECERR_CLIENT_ERROR;

	PEcMailboxCmd pCmd = MailboxAlloc(sizeof(EcMailboxCmd));
	memset(pCmd, 0, sizeof(EcMailboxCmd));	

	pCmd->index = nIndex;
	pCmd->subIndex = nSubIndex;
	pCmd->length = cbRdLength;
	pCmd->cmdId = nCmdId;
	pCmd->type = EC_MAILBOX_CMD_UPLOAD;
	pCmd->invokeId = nInvokeId;
	pCmd->addr.port = nSlave;

	return SyncSendMailboxCmdEcSyncClient(pEcSyncClient, pCmd, cbRdLength, pRdData, cbRead);
}


//////////////////////////////////////////////////////////////////////////////////////////
long SyncDownloadReqEcSyncClient(PEcSyncClient pEcSyncClient,unsigned long nInvokeId, unsigned short nSlave, unsigned long nCmdId, unsigned short nIndex, unsigned char nSubIndex, unsigned long cbWrLength, unsigned char *pWrData)
{
	if( cbWrLength > 0 && pWrData == NULL)
		return ECERR_CLIENT_INVALIDPARM;

	if( pEcSyncClient->m_pMailbox == NULL )	
		return ECERR_CLIENT_ERROR;

	PEcMailboxCmd pCmd = MailboxAlloc(sizeof(EcMailboxCmd)+cbWrLength);
	unsigned char *pData = (unsigned char*) ENDOF(pCmd);
	unsigned long cbRead;

	memset(pCmd, 0, sizeof(EcMailboxCmd));	

	pCmd->index = nIndex;
	pCmd->subIndex = nSubIndex;
	pCmd->length = cbWrLength;
	pCmd->cmdId = nCmdId;
	pCmd->type = EC_MAILBOX_CMD_DOWNLOAD;
	pCmd->invokeId = nInvokeId;
	pCmd->addr.port = nSlave;

	memcpy(pData, pWrData, cbWrLength);

	return SyncSendMailboxCmdEcSyncClient(pEcSyncClient, pCmd, 0, NULL, cbRead);
}

long SyncUploadReqEcSyncClient(PEcSyncClient pEcSyncClient, unsigned long nInvokeId, unsigned short nSlave, unsigned long nCmdId, unsigned long nIndexOffset, unsigned long cbRdLength, unsigned char *pRdData, unsigned long &cbRead)
{
	if( cbRdLength > 0 && pRdData == NULL)
		return ECERR_CLIENT_INVALIDPARM;

	if( pEcSyncClient->m_pMailbox == NULL )	
		return ECERR_CLIENT_ERROR;

	PEcMailboxCmd pCmd = MailboxAlloc(sizeof(EcMailboxCmd));
	memset(pCmd, 0, sizeof(EcMailboxCmd));	

	pCmd->indexOffset = nIndexOffset;
	pCmd->length = cbRdLength;
	pCmd->cmdId = nCmdId;
	pCmd->type = EC_MAILBOX_CMD_UPLOAD;
	pCmd->invokeId = nInvokeId;
	pCmd->addr.port = nSlave;

	return SyncSendMailboxCmdEcSyncClient(pEcSyncClient, pCmd, cbRdLength, pRdData, cbRead);
}

//////////////////////////////////////////////////////////////////////////////////////////
long SyncDownloadReqEcSyncClient(PEcSyncClient pEcSyncClient,unsigned long nInvokeId, unsigned short nSlave, unsigned long nCmdId, unsigned long nIndexOffset, unsigned long cbWrLength, unsigned char *pWrData)
{
	if( cbWrLength > 0 && pWrData == NULL)
		return ECERR_CLIENT_INVALIDPARM;

	if( pEcSyncClient->m_pMailbox == NULL )	
		return ECERR_CLIENT_ERROR;

	PEcMailboxCmd pCmd = MailboxAlloc(sizeof(EcMailboxCmd)+cbWrLength);
	unsigned char *pData = (unsigned char*) ENDOF(pCmd);
	unsigned long cbRead;

	memset(pCmd, 0, sizeof(EcMailboxCmd));	

	pCmd->indexOffset = nIndexOffset;
	pCmd->length = cbWrLength;
	pCmd->cmdId = nCmdId;
	pCmd->type = EC_MAILBOX_CMD_DOWNLOAD;
	pCmd->invokeId = nInvokeId;
	pCmd->addr.port = nSlave;

	memcpy(pData, pWrData, cbWrLength);

	return SyncSendMailboxCmdEcSyncClient(pEcSyncClient, pCmd, 0, NULL, cbRead);
}
	
//////////////////////////////////////////////////////////////////////////////////////////
long SyncReadWriteReqEcSyncClient(PEcSyncClient pEcSyncClient, unsigned long nInvokeId,  unsigned short nSlave, unsigned long nCmdId, unsigned long nIndexOffset, unsigned long cbRdLength, unsigned char *pRdData, 
	unsigned long cbWrLength, unsigned char *pWrData, unsigned long &cbRead)
{		
	if( cbRdLength > 0 && pRdData == NULL)
		return ECERR_CLIENT_INVALIDPARM;

	if( cbWrLength > 0 && pWrData == NULL)
		return ECERR_CLIENT_INVALIDPARM;

	if( pEcSyncClient->m_pMailbox == NULL )	
		return ECERR_CLIENT_ERROR;	

	PEcMailboxCmdRW pCmd = (PEcMailboxCmdRW)MailboxAlloc(sizeof(EcMailboxCmdRW)+cbWrLength);
	unsigned char *pData = (unsigned char*) ENDOF(pCmd);			

	memset(pCmd, 0, sizeof(EcMailboxCmd));	
	pCmd->indexOffset = nIndexOffset;
	pCmd->length = cbRdLength;
	pCmd->writeLength = cbWrLength;
	pCmd->cmdId = nCmdId;
	pCmd->type = EC_MAILBOX_CMD_READWRITE;
	pCmd->invokeId = nInvokeId;
	pCmd->addr.port = nSlave;
				
	memcpy(pData, pWrData, cbWrLength);

	return SyncSendMailboxCmdEcSyncClient(pEcSyncClient, (PEcMailboxCmd)pCmd, cbRdLength, pRdData, cbRead);
}

