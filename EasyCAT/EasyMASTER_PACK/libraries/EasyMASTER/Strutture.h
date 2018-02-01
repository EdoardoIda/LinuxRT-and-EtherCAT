

#pragma pack( 1 )

#include "fifo.h"
#include "Hashtable.h"



typedef enum EEC_DC_TIMING_STATE
{
	EC_DC_TIMING_INIT								= 1,
	EC_DC_TIMING_SCAN_LINK_STATUS,
	EC_DC_TIMING_READ_LINK_STATUS,
	EC_DC_TIMING_READ_TIMING,
	EC_DC_TIMING_CALC_TIMING,
	EC_DC_TIMING_WRITE_CLOCK_DELAY,
	EC_DC_TIMING_WRITE_SYSTIME_OFFS,
	EC_DC_TIMING_WRITE_SPEED_START,
	EC_DC_TIMING_CHECK_REFCLOCK,
	EC_DC_TIMING_SEND_CLOCK,
	EC_DC_TIMING_DONE,
	EC_DC_TIMING_IDLE,
	EC_DC_TIMING_ONLINE_INIT,
	EC_DC_TIMING_CHECK_BUILD_ARMW,
} EC_DC_TIMING_STATE;

typedef	enum EECMI_DC
{	
	ECMI_DC_INIT				= 0x20000,	
} ECMI_DC;

typedef union _LARGE_INTEGER {
  struct {
    unsigned long LowPart;
    long  HighPart;
  };
  struct {
    unsigned long LowPart;
    long  HighPart;
  } u;
  int64_t QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef  struct _TimeoutHelper
{
	//LARGE_INTEGER	    m_timeElapse;
	long	    m_timeElapse;									   
} TimeoutHelper, *PTimeoutHelper;


typedef struct  _IoImage
{
	unsigned char *		m_pIn;
	unsigned char *		m_pOut;
	unsigned long		m_nIn;
	unsigned long		m_nOut;
} IoImage, *PIoImage;


struct timeval {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
};

/*!
  \brief Packet header.

  This structure defines the header associated with every packet delivered to the application.
*/
struct bpf_hdr {
	struct timeval	bh_tstamp;	///< The timestamp associated with the captured packet. 
								///< It is stored in a TimeVal structure.
	unsigned int	bh_caplen;	///< Length of captured portion. The captured portion <b>can be different</b>
								///< from the original packet, because it is possible (with a proper filter)
								///< to instruct the driver to capture only a portion of the packets.
	unsigned int	bh_datalen;	///< Original length of packet
	unsigned short	bh_hdrlen;		///< Length of bpf header (this struct plus alignment padding). In some cases,
								///< a padding could be added between the end of this structure and the packet
								///< data for performance reasons. This filed can be used to retrieve the actual data 
								///< of the packet.
};


typedef struct TECMBSLAVE_COE_SUPPORT
{
	CFiFoListDyn<PEcMailboxCmd>*	pPend;
	struct ssafe
	{
		unsigned short				bValid;
		ETHERCAT_SDO_HEADER			sdo;
		PEcMailboxCmd				pCmd;				// MailboxAlloc
		unsigned char *				pRetData;			// MailboxAlloc
		unsigned long				nRetData;
		unsigned long				nOffset;
		unsigned short				retry;
		void *						timeout;
	} safe;
} ECMBSLAVE_COE_SUPPORT, *PECMBSLAVE_COE_SUPPORT;

///////////////////////////////////////////////////////////////////////////////
typedef struct TECMBSLAVE_FOE_SUPPORT
{
	CFiFoListDyn<PEcMailboxCmd>*									pPend;
	struct ssafe
	{
		unsigned long				bValid		: 1;
		unsigned long				bNewCmd		: 1;
		unsigned long				bRead		: 1;
		unsigned long				bDataPend	: 1;
		unsigned long				bBusy		: 1;
		unsigned long				bLastRead	: 1;
		PEcMailboxCmd				pRet;					// MailboxAlloc
		PEcMailboxCmdRW				pCmd;					// MailboxAlloc
		unsigned char *				pData;					// MailboxAlloc
		unsigned long				nOffset;
		unsigned short				retry;
		unsigned short				nData;				
		unsigned long				nPacketNo;				
		void *						timeout;
		void *						busy;
		ETHERCAT_FOE_BUSY_INFO	busyInfo;
	} safe;
	unsigned long							fileHnd;
} ECMBSLAVE_FOE_SUPPORT, *PECMBSLAVE_FOE_SUPPORT;

typedef struct TECMBSLAVE_SOE_CMD_INFO
{
	PEcMailboxCmd				pCmd;
	void *						timeout;
	ECMBSLAVE_SOE_CMD_STATE	state;
	unsigned long				result;
} ECMBSLAVE_SOE_CMD_INFO, *PECMBSLAVE_SOE_CMD_INFO;


typedef struct TECMBSLAVE_SOE_SUPPORT
{
	CFiFoListDyn<PEcMailboxCmd>*					pPend;	
	CHashTableDyn<unsigned short, ECMBSLAVE_SOE_CMD_INFO>*	pCmds;
	struct ssafe
	{
		unsigned short					bValid;
		PEcMailboxCmd					pCmd;				// MailboxAlloc
		unsigned char *					pRetData;			// MailboxAlloc
		unsigned long					nRetData;
		unsigned long					retry;
		void *							timeout;

		ETHERCAT_SOE_HEADER				soe;
		unsigned short					nIDN;
		unsigned short					nOffset;
	} safe;
} ECMBSLAVE_SOE_SUPPORT, *PECMBSLAVE_SOE_SUPPORT;




typedef struct  _EcMailbox
{
	EcMailboxAddr		m_addr;
	void *				m_pOwner;
	void *				m_pClient;
} EcMailbox, *PEcMailbox;



typedef struct TECAT_SLAVECMD_INFO
{
	void *				pSlave;
	unsigned long		invokeId;
} ECAT_SLAVECMD_INFO, *PECAT_SLAVECMD_INFO;

 
typedef struct TECAT_SLAVEFRAME_INFO
{
	LIST_ENTRY_AB				list; 
	ETHERNET_88A4_MAX_FRAME		        frame;
	PETYPE_EC_HEADER			pLastHead;
	ECAT_SLAVECMD_INFO			cmdInfo[MAX_SLAVECMD];
	unsigned long				nInfo;
	unsigned long				nSpace;
	void *					timeout;
	unsigned short				retry;
	unsigned char				idx;
} ECAT_SLAVEFRAME_INFO, *PECAT_SLAVEFRAME_INFO;



typedef	enum {simpleST,portST,routerST,ax2000ST, el67xxST} SLAVE_TYPE;

typedef struct _EcSlave
{


	SLAVE_TYPE					m_slaveType;
	void *						m_pMaster;
	unsigned short				m_physAddr;
	unsigned short				m_autoIncAddr;
	unsigned short				m_currState;
	unsigned short				m_reqState;
	char						m_szName[ECAT_DEVICE_NAMESIZE+1];		
	void *						m_tInitCmds;
	unsigned short				m_rInitCmds;
	unsigned short				m_cInitCmds;
	unsigned short				m_nInitCmds;
	PEcInitCmdDesc				m_pInitCmds;
	PEcInitCmdDesc*				m_ppInitCmds;

	void *						m_pEcSlaves[EC_MAX_PORTS];

	unsigned short				m_initcmdLen;
	
	// Inizio area DC
	unsigned short				m_prevPhysAddr;	
	unsigned short				m_prevPort;	

	unsigned short				m_bDcRefClock;
	long						m_nDcLineDelay[ECAT_ESC_PORTS_MAX];
	unsigned long				m_nDcRefClockDelay928;
	short						m_nDcExecDelay;	
	int							m_bDcSupport;
	int							m_bDc64Support;
	unsigned long				m_nDcPortDiff[ECAT_ESC_PORTS_MAX];	// 0 = unused, 1 = DB, 2 = BC, 3 = AD
	short						m_nDcPortDelayT[ECAT_ESC_PORTS_MAX];
	short						m_nDcPortDelayR[ECAT_ESC_PORTS_MAX];
	unsigned short				m_regDlStatus;
	unsigned long				m_nDcPortCnt;
	unsigned long				m_nDcTimings;
	int							m_bDcTimingPending;
	uint64_t					m_nDcSysTimeOffs920;
	unsigned long				m_dcReceiveTime[ECAT_ESC_PORTS_MAX];
	// Fine area DC

	unsigned short				m_type;
	void *						m_pEcMbSlave;

} EcSlave, *PEcSlave;
 

typedef struct  _EcMbSlave
{
 
	PEcSlave					m_pEcSlave;


	PECMBSLAVE_COE_SUPPORT	m_pCoE;
	PECMBSLAVE_FOE_SUPPORT	m_pFoE;
	PECMBSLAVE_SOE_SUPPORT	m_pSoE;
 
	unsigned short				m_actTransition;
	
	unsigned short				m_mbxOStart[2];
	unsigned short				m_mbxOLen[2];
	unsigned short				m_mbxIStart[2];
	unsigned short				m_mbxILen[2];
	
	struct
	{
		unsigned short						m_mbxOutShortSend	: 1;
		unsigned short						m_mbxIdx			: 1;
		unsigned short						m_cycleMbxPolling	: 1;
		unsigned short						m_stateMbxPolling	: 1;
	};

	void *						m_tMbxCmds;
	unsigned short				m_rMbxCmds;
	unsigned short				m_cMbxCmds;
	unsigned short				m_nMbxCmds;
	unsigned short				m_oMbxCmds;
	PEcMailboxCmdDesc			m_pMbxCmds;
	PEcMailboxCmdDesc*			m_ppMbxCmds;
	unsigned int				m_mboxCmdLen;

	unsigned short				m_slaveAddressMBoxState;		// bit offset in logical area
	unsigned short				m_cycleMbxPollingTime;			// cycle time for MBox polling

} EcMbSlave, *PEcMbSlave;




struct EcCycInfo
{
	void *						pEcMaster;	
	ETHERNET_88A4_FRAME			e88A4Frame;
	PEcCycImage					pState;
	PEcCycImage					pCtrl;
	unsigned short				imageOffs[2];
	unsigned short				cmdCnt;	
	ETYPE_VLAN_HEADER			vlanInfo;
	unsigned short				cdlNo;		// 0, 1, 2, 3...   0 = highest prio
	unsigned short				state;
};
typedef EcCycInfo *PEcCycInfo;

typedef struct TEcCmdInfo
{
	ETYPE_EC_HEADER			cmdHeader;
	EcCycInfo*				pFrameInfo; 
	unsigned short			cntSend;
	unsigned short			cntRecv;
	unsigned short			cmdSize;
	unsigned char *			pCycInputs;
	unsigned char *			pCycOutputs;
	unsigned short			imageOffs[2];
	unsigned short			imageSize[2];	
	unsigned short			copyInputs;
	unsigned short			copyOutputs;
	unsigned short			mboxState;
	unsigned short			send;
	unsigned short			cyclic;
	unsigned short			state;
} EcCmdInfo, *PEcCmdInfo;



typedef  struct _EcMaster
{
	unsigned long						m_maxEcCycInfo;
	unsigned long						m_nEcCycInfo;
	unsigned long						m_nEcSlave;
	unsigned long						m_nEcMbSlave;
	unsigned long						m_maxAsyncFrameSize;
	unsigned long						m_logAddressMBoxStates;		// start address of log. area of sync man states
	unsigned long						m_reqState;
	unsigned long						m_ioOpenStateInit; 
	unsigned long						m_cInitCmds;
	unsigned long						m_nInitCmds;
	unsigned long						m_sizeAddressMBoxStates;	// size of log. area of sync man states
	unsigned long						m_maxSlaves;
	unsigned long						m_initcmdLen;

	// Inizio area DC
	EcSlave*							m_pRefClock;
	unsigned long						m_dcSyncMode;
	unsigned long						m_nDcTimingCounter;
	unsigned long						m_nDcTimingSendArmw;
	unsigned long						m_eDcState;
	int									m_enableContinuousMeasuring;	
	// Fine area DC

	char							    m_szName[ECAT_DEVICE_NAMESIZE+1];	
	ETHERNET_ADDRESS					m_macDest;
	ETHERNET_ADDRESS					m_macSrc;
	void * 								m_ipDev;	
	void *								m_tCyclicSend;
	void *								m_tInitCmds;
	CFiFoList<EcAcycFrameInfo, 100>		m_listAcycFrame;
	//CHashTable<unsigned long, PEcCmdInfo, 100>	m_mapEcCmdInfo;

	LIST_ENTRY_AB						m_listFree;
	LIST_ENTRY_AB						m_listPend;
	PECAT_SLAVEFRAME_INFO				m_pInfoBuffer;
	PECAT_SLAVEFRAME_INFO				m_pPend;
	
	
	// slave infos
 	CHashTableDyn<unsigned short, EcSlave*>*	m_pEcSlave;
	
 	EcSlave**							m_ppEcSlave;
 	EcMbSlave**							m_ppEcMbSlave;
	
	
	// master state
	EC_MASTER_STATE						m_currState;

	// master init commands
	PEcInitCmdDesc						m_pInitCmds;
	PEcInitCmdDesc*						m_ppInitCmds;


	// mailbox state evaluation
 	EcMbSlave**							m_ppEcPortMBoxState;
 	CFiFoList<EcMbSlave*, 10>			m_listProcessOnTimer;
	CFiFoList<EcMbSlave*, 100>			m_listReadSlaveMbx;


	PEcCycInfo*							m_ppEcCycInfo;
	EcMailbox							m_mailbox;	

} EcMaster, *PEcMaster;





	


#define       DOSNAMEPREFIX   TEXT("Packet_")	///< Prefix added to the adapters device names to create the WinPcap devices
#define       MAX_LINK_NAME_LENGTH	64			//< Maximum length of the devices symbolic links
#define       NMAX_PACKET 65535



#define ADAPTER_NAME_LENGTH 256 + 12	///<  Maximum length for the name of an adapter. The value is the same used by the IP Helper API.
#define ADAPTER_DESC_LENGTH 128			///<  Maximum length for the description of an adapter. The value is the same used by the IP Helper API.
#define MAX_MAC_ADDR_LENGTH 8			///<  Maximum length for the link layer address of an adapter. The value is the same used by the IP Helper API.
#define MAX_NETWORK_ADDRESSES 16		///<  Maximum length for the link layer address of an adapter. The value is the same used by the IP Helper API.

class CEcNpfDevice;

typedef  struct _EcDevice
{
	ETHERNET_ADDRESS					m_macAdapter;
	PIoImage 							m_pImage;
	PEcMaster							m_pEcMaster;	
 	void *								m_pCallback;
	bool								m_bOpened;
	unsigned long						m_watchdogStartIo;
	unsigned short						m_bLinkError;
	unsigned short *					m_pDevState;
	unsigned short *					m_pDevCtrl;		
 	CFiFoList<PETHERNET_88A4_FRAME, ETHERNETRTMP_MAXE88A4FIFO>*	m_pFifoE88A4;	
 	CFiFoList<PEcUserFrameInfo, ETHERNETRTMP_MAXECUSERFIFO>*	m_pFifoEcFrame;
 	PEcUserFrameInfo					m_pActiveEcFrame;
	
// 	static unsigned long ReceiverThread(void * lpParameter);
	unsigned long                       m_DummyReceiverThread;
	unsigned long						m_hReceiverThread;
	unsigned long						m_dwThreadId;	
	bool								m_bStopReceiver;
	long								m_lRef;
	char *								m_pszAdapter;
	struct _ADAPTER*					m_pAdapter;			
	CEcNpfDevice*						m_pEcNpfDevice;	



	void *								pTrasmBuffer;
	unsigned int *						pLung;

} EcDevice, *PEcDevice;


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


typedef  struct _EcSyncClient
{
	PEcMailbox			m_pMailbox;
	PEcDevice			m_pDevice;
	unsigned long		m_nTimeout;
	EcSyncDevReq		m_syncDevReq;
	EcSyncMbReq			m_syncMbReq;
	bool				m_bWindowsTimer;

	unsigned long		m_nInvokeId;

} EcSyncClient, *PEcSyncClient;


#pragma pack( )
