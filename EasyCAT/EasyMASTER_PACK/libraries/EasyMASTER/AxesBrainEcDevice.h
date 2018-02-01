

//////////////////////////////////////////////////////////////
///////////////      Utility               ///////////////////
//////////////////////////////////////////////////////////////

unsigned long	EcCmdHash( PETYPE_EC_HEADER pHead);

//void			OsQueryTickCount(LARGE_INTEGER* pTicks);
void			OsQueryTickCount(long  * pTicks);
void			Start(void * pTime, unsigned int timeoutMsec);

bool			IsElapsed(void * pTime);

unsigned char *	GetInputPtr(PIoImage pImage, unsigned long offs, unsigned long size);

unsigned char *	GetOutputPtr(PIoImage pImage, unsigned long offs, unsigned long size);

unsigned char * ProcessDataPtr(PEcDevice  pEcDevice, int inOut, int offs, int size);

PEcMailboxCmd	MailboxAlloc(long size);

void			MailboxFree(PEcMailboxCmd pCmd); 

long			SendMailboxCmd(PEcMailbox pEcMailbox, PEcMailboxCmd pCmd);


//////////////////////////////////////////////////////////////
///////////////      EcDevice              ///////////////////
//////////////////////////////////////////////////////////////


unsigned long	GetOutputSizeEcDevice(PEcDevice  pEcDevice );

unsigned char * GetInputPtrEcDevice(PEcDevice  pEcDevice, unsigned long offs, unsigned long size);

unsigned char * GetOutputPtrEcDevice(PEcDevice  pEcDevice, unsigned long offs, unsigned long size);

int				QueueE88A4CmdEcDevice(PEcDevice pEcDevice, PETHERNET_88A4_FRAME pFrame);		

void			FrameFreeEcDevice(PEcDevice  pEcDevice, void * pData);

void *			FrameAllocEcDevice(PEcDevice  pEcDevice, unsigned long nData);

void				FrameSendAndFreeEcDevice(PEcDevice  pEcDevice, void * pData, unsigned long  nData);

long			SendEcFrameEcDevice(PEcDevice pEcDevice, unsigned long invokeId, unsigned long cbLength, PETYPE_EC_HEADER pFrame);

void			SendQueuedE88A4CmdsEcDevice(PEcDevice pEcDevice, long maxTime=0);

void			SendQueuedEcCmdsEcDevice(PEcDevice pEcDevice, long maxTime=0);

int				OnTimerEcDevice(PEcDevice pEcDevice);

long			StartIoEcDevice(PEcDevice pEcDevice);

long			GetIoStateEcDevice(PEcDevice pEcDevice);




//////////////////////////////////////////////////////////////
///////////////      EcMaster              ///////////////////
//////////////////////////////////////////////////////////////

int		OpenEcMaster( PEcMaster pEcMaster );

int		OnTimerEcMaster( PEcMaster pEcMaster, unsigned long nTickNow );

void	SetSrcMacAddressEcMaster( PEcMaster pEcMaster, ETHERNET_ADDRESS macAdapter );		

void	AddToOnTimerListEcMaster( PEcMaster pEcMaster, PEcMbSlave pSlave );

unsigned short	GetStateMachineEcMaster(PEcMaster pEcMaster);

int		StateMachineEcMaster(PEcMaster pEcMaster, int bOnTimer);

void     SendCycFramesEcMaster( PEcMaster pEcMaster ); 

void	StartInitCmdsEcMaster(PEcMaster pEcMaster,unsigned short stateValue);

void	StartInitCmdsEcMaster(PEcMaster pEcMaster, unsigned short stateValue, EC_MASTER_STATE stateNext);

int		CyclicTimeoutElapsedEcMaster( PEcMaster pEcMaster );

long	EcatCmdFlushEcMaster(PEcMaster pEcMaster); 

long	EcatCmdReqEcMaster( PEcMaster pEcMaster, EcSlave* pSlave, unsigned long invokeId, unsigned char cmd,
						    unsigned short adp, unsigned short ado, unsigned short len, void * pData, 
							unsigned short cnt=0,
							PETHERCAT_MBOX_HEADER pMbox= NULL,
							unsigned int pEthernet = NULL,
							PETHERCAT_CANOPEN_HEADER pCANopen = NULL,
							PETHERCAT_SDO_HEADER pSDO= NULL, 
							PETHERCAT_FOE_HEADER pFoE= NULL, 
							PETHERCAT_SOE_HEADER pSoE= NULL
						  );

long	EcatCmdReqEcMaster(PEcMaster pEcMaster, EcSlave* pSlave, unsigned long invokeId, unsigned char cmd, unsigned short adp, unsigned short ado, unsigned short data=0);

void	EcatCmdResEcMaster(PEcMaster pEcMaster, unsigned long result, unsigned long invokeId, PETYPE_EC_HEADER pHead);

void	CheckAsyncFrameEcMaster(PEcMaster pEcMaster);

int		CheckFrameEcMaster(PEcMaster pEcMaster, PETHERNET_FRAME pFrame, unsigned long nFrame, void * pProtocolBegin, int &bPending);

int		CheckMBoxCmdEcMaster(PEcMaster pEcMaster,PETYPE_EC_HEADER pHead);

long	OnMailboxReceiveEcMaster( PEcMaster pEcMaster,PEcMailboxCmd pCmd );

int		RegisterMBoxPollingEcMaster( PEcMaster pEcMaster, EcMbSlave * pSlave, unsigned short slaveAddressMBoxState, int bUnregister);

PETHERNET_88A4_MAX_FRAME AllocEcFrameEcMaster( PEcMaster pEcMaster, unsigned long size);

void	FreeEcFrameEcMaster( PEcMaster pEcMaster, PETHERNET_88A4_MAX_FRAME pFrame);

void	MailboxResponseEcMaster( PEcMaster pEcMaster, PEcMailboxCmd pCmd);




//////////////////////////////////////////////////////////////
///////////////      EcSlave               ///////////////////
//////////////////////////////////////////////////////////////

void	RequestStateEcSlave(PEcSlave pEcSlave , unsigned short state);

int		StateMachineEcSlave(PEcSlave pEcSlave , short iPino=0);

void	StartInitCmdsEcSlave(PEcSlave pEcSlave, unsigned short transition, short iPino=0);

void	StopInitCmdsEcSlave(PEcSlave pEcSlave);

int		InitCmdsActiveEcSlave(PEcSlave pEcSlave);

void	EcatCmdResEcSlave(PEcSlave pEcSlave, unsigned long result, unsigned long invokeId, PETYPE_EC_HEADER pHead,short iPino);

void	CancellaEcSlave(PEcSlave pEcSlave); 

void	ExecDcTimingEcSlave(PEcSlave pEcSlave, EC_DC_TIMING_STATE dcState);

long	DcCalcDelayAAEcSlave(PEcSlave pEcSlave);

//////////////////////////////////////////////////////////////
///////////////      EcMailboxSlave        ///////////////////
//////////////////////////////////////////////////////////////
int		StateMachineEcMbSlave(PEcMbSlave pEcMbSlave);

void	StartInitCmdsEcMbSlave(PEcMbSlave pEcMbSlave, unsigned short transition);

void	StopInitCmdsEcMbSlave(PEcMbSlave pEcMbSlave);

int	InitCmdsActiveEcMbSlave(PEcMbSlave pEcMbSlave);

void	CancellaEcMbSlave(PEcMbSlave pEcMbSlave); 

int		HasMailBoxEcMbSlave(PEcMbSlave pEcMbSlave);

unsigned short	GetMBoxOutCmdLengthEcMbSlave(PEcMbSlave pEcMbSlave, unsigned short length);

unsigned short	GetFoEDataSizeEcMbSlave(PEcMbSlave pEcMbSlave, int inOut);

unsigned short	GetSoEDataSizeEcMbSlave(PEcMbSlave pEcMbSlave, int inOut);

int		EcatMbxSendCmdReqEcMbSlave(PEcMbSlave pEcMbSlave,
					  void * pData,
					  PETHERCAT_MBOX_HEADER pMbox, 
					  unsigned int pEthernet=NULL,
					  PETHERCAT_CANOPEN_HEADER pCANopen=NULL,
					  PETHERCAT_SDO_HEADER pSDO=NULL,
					  PETHERCAT_FOE_HEADER pFoE=NULL,
					  PETHERCAT_SOE_HEADER pSoE=NULL
					 );
int		CycleMBoxPollingEcMbSlave(PEcMbSlave pEcMbSlave);

void	OnTimerEcMbSlave(PEcMbSlave pEcMbSlave,unsigned long nTickNow);

int		SendFrameToSlaveEcMbSlave(PEcMbSlave pEcMbSlave, PETHERNET_FRAME pData, unsigned long nData);

void	ClearCoESafeEcMbSlave(PEcMbSlave pEcMbSlave);

void	ClearFoESafeEcMbSlave(PEcMbSlave pEcMbSlave);

void	ClearSoESafeEcMbSlave(PEcMbSlave pEcMbSlave);

void	StopCoESafeRequestEcMbSlave(PEcMbSlave pEcMbSlave, unsigned long result, unsigned long abortCode=0, unsigned long nData=0, void * pData=NULL);

void	StopFoESafeRequestEcMbSlave(PEcMbSlave pEcMbSlave, unsigned long result, unsigned long errCode, unsigned long nErrText=0, PCHAR pErrText=NULL);

void	StopSoESafeRequestEcMbSlave(PEcMbSlave pEcMbSlave, unsigned long result, unsigned long nData=0, void * pData=NULL);

int		MailboxResEcMbSlave(PEcMbSlave pEcMbSlave, PEcMailboxCmd pReq, unsigned long nResult=0, unsigned long cbLength=0, void * pData=NULL);

int		MailboxUploadResEcMbSlave(PEcMbSlave pEcMbSlave, PEcMailboxCmd pReq, unsigned long nResult, unsigned long cbLength, void * pData);

int		MailboxDownloadResEcMbSlave(PEcMbSlave pEcMbSlave, PEcMailboxCmd pReq, unsigned long nResult);

long	MailboxReceiveFromMasterEcMbSlave(PEcSlave pEcSlave, PEcMailboxCmd pCmd );

void	MBoxReadFromSlaveEcMbSlave(PEcMbSlave pEcMbSlave);

int		IsCoECmdEcMbSlave( PEcMbSlave pEcMbSlave, PEcMailboxCmd pCmd );

int		IsFoECmdEcMbSlave( PEcMbSlave pEcMbSlave, PEcMailboxCmd pCmd );

int		IsSoECmdEcMbSlave( PEcMbSlave pEcMbSlave, PEcMailboxCmd pCmd );

void	SendQueuedCoECmdsEcMbSlave( PEcMbSlave pEcMbSlave );

void	SendQueuedFoECmdEcMbSlave( PEcMbSlave pEcMbSlave );

void	SendQueuedSoECmdsEcMbSlave( PEcMbSlave pEcMbSlave );

void	ProcessCoEReturningRequestEcMbSlave( PEcMbSlave pEcMbSlave, unsigned short wc, PETHERCAT_MBOX_HEADER pMBox);

void	ProcessFoEReturningRequestEcMbSlave( PEcMbSlave pEcMbSlave, unsigned short wc, PETHERCAT_MBOX_HEADER pMBox);

void	ProcessSoEReturningRequestEcMbSlave( PEcMbSlave pEcMbSlave, unsigned short wc, PETHERCAT_MBOX_HEADER pMBox);

void	ProcessCoEResponseEcMbSlave(PEcMbSlave pEcMbSlave, PETHERCAT_MBOX_HEADER pMBox);

void	ProcessFoEResponseEcMbSlave( PEcMbSlave pEcMbSlave, PETHERCAT_MBOX_HEADER pMBox);

void	ProcessSoEResponseEcMbSlave( PEcMbSlave pEcMbSlave, PETHERCAT_MBOX_HEADER pMBox);

//////////////////////////////////////////////////////////////
///////////////      EcSyncClient         ///////////////////
//////////////////////////////////////////////////////////////

void OnReturningEcFrameEcSyncClient(PEcSyncClient pEcSyncClient, unsigned long nResult, unsigned long nInvokeId, unsigned long cbLength, PETYPE_EC_HEADER pFrame);

void OnMailboxResponseEcSyncClient(PEcSyncClient pEcSyncClient, PEcMailboxCmd pCmd, PEcMailbox  pMailbox);

long SyncSendMailboxCmdEcSyncClient(PEcSyncClient pEcSyncClient,PEcMailboxCmd pCmd, unsigned long cbRdLength, unsigned char *pRdData, unsigned long &cbRead);

long SyncUploadReqEcSyncClient(PEcSyncClient pEcSyncClient,
							   unsigned long nInvokeId, 
							   unsigned short nSlave, 
							   unsigned long nCmdId, 
							   unsigned short nIndex, 
							   unsigned char nSubIndex, 
							   unsigned long cbRdLength, 
							   unsigned char *pRdData, 
							   unsigned long &cbRead);

long SyncDownloadReqEcSyncClient(PEcSyncClient pEcSyncClient,
								 unsigned long nInvokeId, 
								 unsigned short nSlave, 
								 unsigned long nCmdId, 
								 unsigned short nIndex, 
								 unsigned char nSubIndex, 
								 unsigned long cbWrLength, 
								 unsigned char *pWrData);

long SyncUploadReqEcSyncClient(PEcSyncClient pEcSyncClient, 
							   unsigned long nInvokeId, 
							   unsigned short nSlave, 
							   unsigned long nCmdId, 
							   unsigned long nIndexOffset, 
							   unsigned long cbRdLength, 
							   unsigned char *pRdData, 
							   unsigned long &cbRead);

long SyncDownloadReqEcSyncClient(PEcSyncClient pEcSyncClient,
								 unsigned long nInvokeId, 
								 unsigned short nSlave, 
								 unsigned long nCmdId, 
								 unsigned long nIndexOffset, 
								 unsigned long cbWrLength, 
								 unsigned char *pWrData);

long SyncReadWriteReqEcSyncClient(PEcSyncClient pEcSyncClient,
								  unsigned long nInvokeId,
								  unsigned short nSlave,
								  unsigned long nCmdId,
								  unsigned long nIndexOffset,
								  unsigned long cbRdLength,
								  unsigned char *pRdData, 
								  unsigned long cbWrLength, 
								  unsigned char *pWrData, 
								  unsigned long &cbRead);

