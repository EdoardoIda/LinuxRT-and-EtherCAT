


#pragma pack( 1 )


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//  EcGlobal.H

#define HRESULT long

#define	IOSTATE_READY		0
#define	IOSTATE_BUSY		1

#define	ERR_ECERRS					0x0700

#define ECERR_NOERR 0x00
#define ECERR_DEVICE_ERROR				(0x00+ERR_ECERRS) // Error class < device error >
#define ECERR_DEVICE_SRVNOTSUPP			(0x01+ERR_ECERRS) // Service is not supported
#define ECERR_DEVICE_INVALIDCMD			(0x02+ERR_ECERRS) // invalid index
#define ECERR_DEVICE_INVALIDINDEX		(0x03+ERR_ECERRS) // invalid offset
#define ECERR_DEVICE_INVALIDACCESS		(0x04+ERR_ECERRS) // reading/writing not permitted
#define ECERR_DEVICE_INVALIDSIZE		(0x05+ERR_ECERRS) // parameter size not correct
#define ECERR_DEVICE_INVALIDDATA		(0x06+ERR_ECERRS) // invalid parameter value(s)
#define ECERR_DEVICE_NOTREADY			(0x07+ERR_ECERRS) // device is not in a ready state
#define ECERR_DEVICE_BUSY				(0x08+ERR_ECERRS) // device is busy
#define ECERR_DEVICE_INVALIDCONTEXT		(0x09+ERR_ECERRS) // invalid context
#define ECERR_DEVICE_NOMEMORY			(0x0A+ERR_ECERRS) // out of memory
#define ECERR_DEVICE_INVALIDPARM		(0x0B+ERR_ECERRS) // invalid parameter value(s)
#define ECERR_DEVICE_NOTFOUND			(0x0C+ERR_ECERRS) // not found (files, ...)
#define ECERR_DEVICE_SYNTAX				(0x0D+ERR_ECERRS) // syntax error in command or file
#define ECERR_DEVICE_INVALIDSTATE		(0x12+ERR_ECERRS) // device is in invalid state
#define ECERR_DEVICE_NOMOREHDLS			(0x16+ERR_ECERRS) // no more handles	
#define ECERR_DEVICE_TIMEOUT			(0x19+ERR_ECERRS) // device has a timeout
#define ECERR_DEVICE_OPENFAILED			(0x20+ERR_ECERRS) // 
#define ECERR_DEVICE_SENDFAILED			(0x21+ERR_ECERRS)
#define ECERR_DEVICE_INSERTMAILBOX		(0x22+ERR_ECERRS)
#define ECERR_DEVICE_INVALIDOFFSET		(0x23+ERR_ECERRS) // invalid subindex
#define ECERR_DEVICE_UNKNOWNMAILBOXCMD	(0x24+ERR_ECERRS)
#define ECERR_DEVICE_ACCESSDENIED		(0x25+ERR_ECERRS)
#define ECERR_DEVICE_INVALIDARRAYIDX	(0x26+ERR_ECERRS)
#define ECERR_DEVICE_EXISTS				(0x27+ERR_ECERRS)
#define ECERR_DEVICE_INVALIDADDR		(0x28+ERR_ECERRS)
#define ECERR_DEVICE_INCOMPATIBLE		(0x29+ERR_ECERRS)
#define ECERR_DEVICE_ABORTED			(0x2a+ERR_ECERRS) // request is aborted
#define ECERR_DEVICE_PENDING			(0x2b+ERR_ECERRS) // request is aborted


#define ECERR_CLIENT_ERROR					(0x100+ERR_ECERRS)
#define ECERR_CLIENT_TIMEOUT				(0x101+ERR_ECERRS)
#define ECERR_CLIENT_INVALIDPARM			(0x102+ERR_ECERRS) 
#define ECERR_CLIENT_INVALIDSIZE			(0x103+ERR_ECERRS) 

#define EC_E_ERROR						((HRESULT)0x98110000+ECERR_DEVICE_ERROR)						// Error class < device error >
#define EC_E_SRVNOTSUPP					((HRESULT)0x98110000+ECERR_DEVICE_SRVNOTSUPP)				// Service is not supported by server
#define EC_E_INVALIDINDEX 				((HRESULT)0x98110000+ECERR_DEVICE_INVALIDINDEX)				// invalid index
#define EC_E_INVALIDOFFSET				((HRESULT)0x98110000+ECERR_DEVICE_INVALIDOFFSET)			// invalid subindex
#define EC_E_INVALIDCMD					((HRESULT)0x98110000+ECERR_DEVICE_INVALIDCMD)				// invalid subindex
#define EC_E_INVALIDACCESS				((HRESULT)0x98110000+ECERR_DEVICE_INVALIDACCESS)			// reading/writing not permitted
#define EC_E_INVALIDSIZE				((HRESULT)0x98110000+ECERR_DEVICE_INVALIDSIZE)				// parameter size not correct
#define EC_E_INVALIDDATA				((HRESULT)0x98110000+ECERR_DEVICE_INVALIDDATA)				// invalid parameter value(s)
#define EC_E_NOTREADY					((HRESULT)0x98110000+ECERR_DEVICE_NOTREADY)					// device is not in a ready state
#define EC_E_BUSY						((HRESULT)0x98110000+ECERR_DEVICE_BUSY)						// device is busy
#define EC_E_INVALIDCONTEXT				((HRESULT)0x98110000+ECERR_DEVICE_INVALIDCONTEXT)			// invalid context 
#define EC_E_NOMEMORY					((HRESULT)0x98110000+ECERR_DEVICE_NOMEMORY)					// out of memory
#define EC_E_INVALIDPARM				((HRESULT)0x98110000+ECERR_DEVICE_INVALIDPARM)				// invalid parameter value(s)
#define EC_E_NOTFOUND					((HRESULT)0x98110000+ECERR_DEVICE_NOTFOUND)					// not found (files, ...)
#define EC_E_SYNTAX						((HRESULT)0x98110000+ECERR_DEVICE_SYNTAX)						// syntax error in comand or file
#define EC_E_INVALIDSTATE				((HRESULT)0x98110000+ECERR_DEVICE_INVALIDSTATE)				// server is in invalid state
#define EC_E_NOMOREHDLS					((HRESULT)0x98110000+ECERR_DEVICE_NOMOREHDLS)				// no more notification handles	
#define EC_E_TIMEOUT					((HRESULT)0x98110000+ECERR_DEVICE_TIMEOUT)				
#define EC_E_OPENFAILED					((HRESULT)0x98110000+ECERR_DEVICE_OPENFAILED)			
#define EC_E_SENDFAILED					((HRESULT)0x98110000+ECERR_DEVICE_SENDFAILED)			
#define EC_E_INSERTMAILBOX				((HRESULT)0x98110000+ECERR_DEVICE_INSERTMAILBOX)				
#define EC_E_UNKNOWNMAILBOXCMD			((HRESULT)0x98110000+ECERR_DEVICE_UNKNOWNMAILBOXCMD)				
#define EC_E_ACCESSDENIED				((HRESULT)0x98110000+ECERR_DEVICE_ACCESSDENIED)				
#define EC_E_INVALIDARRAYIDX			((HRESULT)0x98110000+ECERR_DEVICE_INVALIDARRAYIDX)				
#define EC_E_EXISTS						((HRESULT)0x98110000+ECERR_DEVICE_EXISTS)				
#define EC_E_INVALIDADDR				((HRESULT)0x98110000+ECERR_DEVICE_INVALIDADD)				
#define EC_E_INCOMPATIBLE				((HRESULT)0x98110000+ECERR_DEVICE_INCOMPATIBLE)				
#define EC_E_ABORTED					((HRESULT)0x98110000+ECERR_DEVICE_ABORTED)				
#define EC_E_PENDING					((HRESULT)0x98110000+ECERR_DEVICE_PENDING)				

#define EC_E_CLIENT_ERROR				((HRESULT)0x98110000+ECERR_CLIENT_ERROR)				
#define EC_E_CLIENT_TIMEOUT				((HRESULT)0x98110000+ECERR_CLIENT_TIMEOUT)				
#define EC_E_CLIENT_INVALIDPARM			((HRESULT)0x98110000+ECERR_CLIENT_INVALIDPARM)				
#define EC_E_CLIENT_INVALIDSIZE			((HRESULT)0x98110000+ECERR_CLIENT_INVALIDSIZE)				

#define EC_LOG_MSGTYPE_HINT			0x001
#define EC_LOG_MSGTYPE_WARN			0x002
#define EC_LOG_MSGTYPE_ERROR		0x004
#define EC_LOG_MSGTYPE_LOG			0x010

#define STATUS_PENDING 0x00000103

#define BYTE byte
#define PVOID void *
#define VOID void
#define PCHAR char *
#define WORD word
#define LONGLONG int64_t
#define ULONGLONG uint64_t
#define USHORT ushort
#define UNALIGNED 
#define ULONG ulong
#define PULONG ulong *

// abstract iteration position
struct __POSITION { };
typedef __POSITION* POSITION;

//typedef	PVOID		POSITION;

#define KIRQL char

#define EC_DECLARE_CRITSEC	
#define EC_INIT_CRITSEC	
#define EC_DELETE_CRITSEC

  

#define EC_DISABLE_IRQS()
#define EC_ENABLE_IRQS()

#define	EC_HEAD_IDX_CYCLIC_START	0x01


#define ENDOF(p)				((p)+1)
#define	safe_delete(p)			{if (p) {delete (p); (p) = NULL;}}
#define	safe_delete_a(p)		{if (p) {delete [] (p); (p) = NULL;}}

//
//  Doubly-linked list manipulation routines.  Implemented as macros
//  but logically these are procedures.
//

typedef struct _LIST_ENTRY_AB 
{
   struct _LIST_ENTRY_AB *Flink;
   struct _LIST_ENTRY_AB *Blink;
} LIST_ENTRY_AB, *PLIST_ENTRY_AB;

//
//  VOID
//  InitializeListHead(
//      PLIST_ENTRY_AB ListHead
//      );
//

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY_AB ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

//
//  PLIST_ENTRY_AB
//  RemoveHeadList(
//      PLIST_ENTRY_AB ListHead
//      );
//

#define RemoveHeadListAB(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryListAB((ListHead)->Flink)}


//
//  VOID
//  RemoveEntryList(
//      PLIST_ENTRY_AB Entry
//      );
//


#define RemoveEntryListAB(Entry) {\
    PLIST_ENTRY_AB _EX_Blink;\
    PLIST_ENTRY_AB _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY_AB ListHead,
//      PLIST_ENTRY_AB Entry
//      );
//

#define InsertTailListAB(ListHead,Entry) {\
    PLIST_ENTRY_AB _EX_Blink;\
    PLIST_ENTRY_AB _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// EthernetServices.h

#define HIBYTE(w)   ((unsigned char) (((unsigned int) (w) >> 8) & 0xFF))
#define LOBYTE(w)   ((unsigned char) (w))

#ifndef	SWAPWORD
 #define  SWAPWORD(w)							(HIBYTE((w)) | (LOBYTE((w)) << 8))
#endif

#define	ETHERNET_FRAME_TYPE_VLAN		0x8100
#define	ETHERNET_FRAME_TYPE_BKHF		0x88A4


#define	ETHERNET_FRAME_TYPE_VLAN_SW		SWAPWORD(ETHERNET_FRAME_TYPE_VLAN)	
#define	ETHERNET_FRAME_TYPE_BKHF_SW		SWAPWORD(ETHERNET_FRAME_TYPE_BKHF)


#define	ETHERNET_MAX_FRAME_LEN			1514
#define	ETHERNET_MAX_VLAN_FRAME_LEN	        1518
#define	ETHERNET_MAX_FRAMEBUF_LEN		1536




///////////////////////////////////////////////////////////////////////////////
typedef struct TETHERNET_ADDRESS
{
	unsigned char b[6];

} ETHERNET_ADDRESS, *PETHERNET_ADDRESS;
#define	ETHERNET_ADDRESS_LEN	sizeof(ETHERNET_ADDRESS)

const	ETHERNET_ADDRESS	BroadcastEthernetAddress={0xff,0xff,0xff,0xff,0xff,0xff};
const	ETHERNET_ADDRESS	NullEthernetAddress={0,0,0,0,0,0};

///////////////////////////////////////////////////////////////////////////////
typedef struct TETHERNET_FRAME 
{
	ETHERNET_ADDRESS	Destination;
	ETHERNET_ADDRESS	Source;
	unsigned short				FrameType;			// in host-order
} ETHERNET_FRAME, *PETHERNET_FRAME;

#define	ETHERNET_FRAME_LEN		sizeof(ETHERNET_FRAME)
#define	ETHERNET_FRAMETYPE_LEN	sizeof(unsigned short)

///////////////////////////////////////////////////////////////////////////////
typedef struct TETYPE_VLAN_HEADER
{
	unsigned short				VLanType;			// 0x8100 in host-order (0x0081)
	unsigned short				VLanIdH		: 4;		
	unsigned short				Reserved	: 1;		
	unsigned short				Priority	: 3;	// 0 = lowest, 7 highest
	unsigned short				VLanIdL		: 8;		
} ETYPE_VLAN_HEADER, *PETYPE_VLAN_HEADER;

#define	ETYPE_VLAN_HEADER_LEN	sizeof(ETYPE_VLAN_HEADER)

#define	VLAN_PRIO_E88A4_IO	6

///////////////////////////////////////////////////////////////////////////////
typedef struct TETHERNET_VLAN_FRAME 
{
	ETHERNET_ADDRESS	Destination;
	ETHERNET_ADDRESS	Source;
	ETYPE_VLAN_HEADER	VLan;
	unsigned short				FrameType;			// in host-order
} ETHERNET_VLAN_FRAME, *PETHERNET_VLAN_FRAME;
#define	ETHERNET_VLAN_FRAME_LEN	sizeof(ETHERNET_VLAN_FRAME)

#define	FRAMETYPE_PTR(p)			(((unsigned short*)p)[6]==ETHERNET_FRAME_TYPE_VLAN_SW ? &((unsigned short*)p)[8] : &((unsigned short*)p)[6])

#define	ENDOF_ETHERNET_FRAME(p)		ENDOF(FRAMETYPE_PTR(p))

///////////////////////////////////////////////////////////////////////////////
#define	ETYPE_88A4_TYPE_ECAT	1		// ECAT header follows
#define	ETYPE_88A4_TYPE_ADS		2		// ADS header follows
#define	ETYPE_88A4_TYPE_IO		3		// IO process image follows directly
#define	ETYPE_88A4_TYPE_NV		4		// Network Variables
#define	ETYPE_88A4_TYPE_CANOPEN	5		// ETHERCAT_CANOPEN_HEADER follows

typedef struct TETYPE_88A4_HEADER
{
	unsigned short	Length		: 11;		// following bytes
	unsigned short	Reserved	: 1;		
	unsigned short	Type		: 4;		// ETYPE_88A4_TYPE_xxx
} ETYPE_88A4_HEADER, *PETYPE_88A4_HEADER;

#define	ETYPE_88A4_HEADER_LEN	sizeof(ETYPE_88A4_HEADER)

///////////////////////////////////////////////////////////////////////////////
typedef struct TETHERNET_88A4_FRAME
{
	ETHERNET_FRAME			Ether;
	ETYPE_88A4_HEADER		E88A4;
} ETHERNET_88A4_FRAME, *PETHERNET_88A4_FRAME;

#define	ETHERNET_88A4_FRAME_LEN	sizeof(ETHERNET_88A4_FRAME)

#define	SIZEOF_88A4_FRAME(p)	(sizeof(ETHERNET_88A4_FRAME)+((PETHERNET_88A4_FRAME)(p))->E88A4.Length)
#define	ENDOF_88A4_FRAME(p)	((PETHERNET_88A4_FRAME)&(((unsigned char*)(p))[SIZEOF_88A4_FRAME(p)]))


///////////////////////////////////////////////////////////////////////////////
typedef struct TETHERNET_88A4_VLAN_FRAME
{
	ETHERNET_VLAN_FRAME	Ether;
	ETYPE_88A4_HEADER		E88A4;
} ETHERNET_88A4_VLAN_FRAME, *PETHERNET_88A4_VLAN_FRAME;

#define	ETHERNET_88A4_VLAN_FRAME_LEN	sizeof(ETHERNET_88A4_VLAN_FRAME)

#define	SIZEOF_88A4_VLAN_FRAME(p)	(sizeof(ETHERNET_88A4_VLAN_FRAME)+((PETHERNET_88A4_VLAN_FRAME)(p))->E88A4.Length)
#define	ENDOF_88A4_VLAN_FRAME(p)	((PETHERNET_88A4_VLAN_FRAME)&(((unsigned char*)(p))[SIZEOF_88A4_VLAN_FRAME(p)]))

///////////////////////////////////////////////////////////////////////////////

typedef enum 
{
	EC_CMD_TYPE_NOP		= 0,
	EC_CMD_TYPE_APRD	= 1,
	EC_CMD_TYPE_APWR	= 2,
	EC_CMD_TYPE_APRW	= 3,
	EC_CMD_TYPE_FPRD	= 4,
	EC_CMD_TYPE_FPWR	= 5,
	EC_CMD_TYPE_FPRW	= 6,
	EC_CMD_TYPE_BRD		= 7,
	EC_CMD_TYPE_BWR		= 8,
	EC_CMD_TYPE_BRW		= 9,
	EC_CMD_TYPE_LRD		= 10,
	EC_CMD_TYPE_LWR		= 11,
	EC_CMD_TYPE_LRW		= 12,
	EC_CMD_TYPE_ARMW	= 13,
	EC_CMD_TYPE_EXT		= 255,
} EC_CMD_TYPE;


///////////////////////////////////////////////////////////////////////////////
#define	EC_HEAD_IDX_ACYCLIC_MASK	0x80
#define	EC_HEAD_IDX_SLAVECMD		0x80
#define	EC_HEAD_IDX_EXTERN_VALUE	0xFF

typedef struct TETYPE_EC_HEADER
{
	union
	{
		struct
		{
			unsigned char	cmd;
			unsigned char	idx;
		};
		unsigned short cmdIdx;
	};
	union
	{
		struct
		{
			unsigned short	adp;
			unsigned short	ado;
		};
		unsigned long laddr;
	};
	union
	{
		struct
		{
			unsigned short	len : 11;
			unsigned short	res : 4;
			unsigned short	next: 1;
		};
		unsigned short length;
	};
	unsigned short irq;
} ETYPE_EC_HEADER, *PETYPE_EC_HEADER;

#define	ETYPE_EC_HEADER_LEN		sizeof(ETYPE_EC_HEADER)
#define	ETYPE_EC_CNT_LEN		sizeof(unsigned short)
#define	ETYPE_EC_OVERHEAD		(ETYPE_EC_HEADER_LEN+ETYPE_EC_CNT_LEN)
											
#define	ETYPE_EC_CMD_LEN(p)			(ETYPE_EC_OVERHEAD+((PETYPE_EC_HEADER)p)->len)
#define	ETYPE_EC_CMD_COUNTPTR(p)	((unsigned short*)&(((unsigned char*)p)[(ETYPE_EC_HEADER_LEN+((PETYPE_EC_HEADER)p)->len)]))
#define	ETYPE_EC_CMD_COUNT(p)		(*((unsigned short*)&(((unsigned char*)p)[(ETYPE_EC_HEADER_LEN+((PETYPE_EC_HEADER)p)->len)])))
#define	ETYPE_EC_CMD_DATA(p)		(*((unsigned short*)&(((unsigned char*)p)[ETYPE_EC_HEADER_LEN])))
#define	ETYPE_EC_CMD_DATAPTR(p)		(&(((unsigned char*)p)[ETYPE_EC_HEADER_LEN]))
#define	NEXT_EcHeader(p)			((PETYPE_EC_HEADER)&((unsigned char*)p)[((PETYPE_EC_HEADER)p)->len + ETYPE_EC_OVERHEAD])


///////////////////////////////////////////////////////////////////////////////
typedef struct TETYPE_88A4_IO_HEADER
{
	union
	{
		struct
		{
			unsigned short		DisableMultiTasking		: 1;
			unsigned short		Reserved				: 15;
			unsigned short		InvokeId;
		} ctrl;
		struct
		{
			unsigned short		Reserved1				: 8;
			unsigned short		InvalidOutputLength		: 1;
			unsigned short		Reserved2				: 7;
			unsigned short		InvokeId;
		} state;
		unsigned long		Head;
	};
} ETYPE_88A4_IO_HEADER, *PETYPE_88A4_IO_HEADER;

#define	ETYPE_88A4_IO_HEADER_LEN	sizeof(ETYPE_88A4_IO_HEADER)

///////////////////////////////////////////////////////////////////////////////
typedef struct TETHERNET_88A4_MAX_HEADER
{
	ETYPE_88A4_HEADER		E88A4;
	union
	{
		struct
		{	// ETYPE_88A4_TYPE_ECAT
			ETYPE_EC_HEADER		FirstEcHead;
		};
		struct
		{	// ETYPE_88A4_TYPE_IO
			ETYPE_88A4_IO_HEADER	IoHead;
			unsigned char					IoData[1494];
		};
		unsigned char						Data[ETHERNET_MAX_FRAME_LEN-ETHERNET_88A4_FRAME_LEN];
	};
} ETHERNET_88A4_MAX_HEADER, *PETHERNET_88A4_MAX_HEADER;

///////////////////////////////////////////////////////////////////////////////
typedef struct TETHERNET_88A4_MAX_FRAME
{
	ETHERNET_FRAME			Ether;
	ETYPE_88A4_HEADER		E88A4;
	union
	{
		struct
		{	// ETYPE_88A4_TYPE_ECAT
			ETYPE_EC_HEADER		FirstEcHead;
		};
		struct
		{	// ETYPE_88A4_TYPE_IO
			ETYPE_88A4_IO_HEADER	IoHead;
			unsigned char						IoData[1494];
		};
		unsigned char						Data[ETHERNET_MAX_FRAME_LEN-ETHERNET_88A4_FRAME_LEN];
	};
} ETHERNET_88A4_MAX_FRAME, *PETHERNET_88A4_MAX_FRAME;



///////////////////////////////////////////////////////////////////////////////
typedef struct TEcCmdDesc
{
	ETYPE_EC_HEADER	head;
	unsigned short				cntSend;
	unsigned short				cmdSize;
	unsigned short				imageOffs[2];
	unsigned short				imageSize[2];
	struct
	{
		unsigned short				copyInputs		: 1;
		unsigned short				copyOutputs		: 1;
		unsigned short				mboxState		: 1;
		unsigned short				dcDeployment	: 1;
	};	
	unsigned short			cntRecv;
	unsigned char				state;
	unsigned char				reserved1;
	unsigned short			reserved2;
} EcCmdDesc, *PEcCmdDesc;

///////////////////////////////////////////////////////////////////////////////
typedef struct TEcCycDesc
{
	ETHERNET_ADDRESS		macTarget;		
	unsigned short					size;
	unsigned short					cntCmd;
	unsigned short					syncMapId;
	unsigned short					cdlNo;			// 0, 1, 2, 3...   0 = highest prio
	unsigned short					imageOffs[2];
	unsigned short					imageSize[2];	
	ETYPE_VLAN_HEADER		vlanInfo;
	unsigned char					ecatMaster;
	unsigned char					state;
	unsigned short					reserved2;
} EcCycDesc, *PEcCycDesc;


#define	SIZEOF_EcCycDesc(p)	(sizeof(EcCycDesc) + ((PEcCycDesc)(p))->cntCmd*sizeof(EcCmdDesc))

///////////////////////////////////////////////////////////////////////////////
#define	ECAT_CYCCMD_OUTPUTS		0x0001
#define	ECAT_CYCCMD_INPUTS		0x0002
#define	ECAT_CYCCMD_MBOXSTATE	0x0004

typedef struct TEcCycCmdDesc
{	
	ETYPE_VLAN_HEADER		vlanInfo;
	ETYPE_EC_HEADER			ecHead;
	unsigned short					cnt;
	unsigned short					state;
	unsigned short					purpose;
	unsigned short					cmtLen;	// (excl. \0)
} EcCycCmdDesc, *PEcCycCmdDesc;

#define	SIZEOF_EcCycCmdDesc(p)	(sizeof(EcCycCmdDesc) + ((PEcCycCmdDesc)(p))->cntCmd*sizeof(EcCmdDesc))

///////////////////////////////////////////////////////////////////////////////
#define	ECAT_INITCMD_I_P				0x0001
#define	ECAT_INITCMD_P_S				0x0002
#define	ECAT_INITCMD_P_I				0x0004
#define	ECAT_INITCMD_S_P				0x0008
#define	ECAT_INITCMD_S_O				0x0010
#define	ECAT_INITCMD_S_I				0x0020
#define	ECAT_INITCMD_O_S				0x0040
#define	ECAT_INITCMD_O_P				0x0080
#define	ECAT_INITCMD_O_I				0x0100
#define	ECAT_INITCMD_I_B				0x0200
#define	ECAT_INITCMD_B_I				0x0400


#define	ECAT_INITCMD_I_PB				0x0201
#define	ECAT_INITCMD_BACKTO_I			0x0524
#define	ECAT_INITCMD_BACKTO_P			0x0088
#define	ECAT_INITCMD_BEFORE				0x8000

typedef struct TEcInitCmdDesc
{
	ETYPE_EC_HEADER					ecHead;
	unsigned short					transition;
	unsigned short					cnt;
	unsigned short					cmtLen;		// (excl. \0)
	unsigned char					newCycle			: 1;
	unsigned char					newFrame			: 1;
	unsigned char					validate			: 1;
	unsigned char					validateMask		: 1;
	unsigned char					invokeId;
	unsigned short					timeout;	// in ms
	unsigned short					retries;	
	unsigned short					reserved2;
} EcInitCmdDesc, *PEcInitCmdDesc;

#define	SIZEOF_EcInitCmdDesc(p)	(sizeof(EcInitCmdDesc) + ((PEcInitCmdDesc)(p))->ecHead.len + \
	(((PEcInitCmdDesc)(p))->validate ? ((PEcInitCmdDesc)(p))->ecHead.len : 0) + \
	(((PEcInitCmdDesc)(p))->validateMask ? ((PEcInitCmdDesc)(p))->ecHead.len : 0) + ((PEcInitCmdDesc)(p))->cmtLen + 1)
#define	NEXT_EcInitCmdDesc(p)	(PEcInitCmdDesc)&(((unsigned char*)(p))[(sizeof(EcInitCmdDesc) + ((PEcInitCmdDesc)(p))->ecHead.len + \
	(((PEcInitCmdDesc)(p))->validate ? ((PEcInitCmdDesc)(p))->ecHead.len : 0) + \
	(((PEcInitCmdDesc)(p))->validateMask ? ((PEcInitCmdDesc)(p))->ecHead.len : 0) + ((PEcInitCmdDesc)(p))->cmtLen + 1)])
#define	EcInitCmdDescData(p)		&(((unsigned char*)(p))[sizeof(EcInitCmdDesc)])
#define	EcInitCmdDescVData(p)	&(((unsigned char*)(p))[sizeof(EcInitCmdDesc) + ((PEcInitCmdDesc)(p))->ecHead.len])
#define	EcInitCmdDescVMData(p)	&(((unsigned char*)(p))[sizeof(EcInitCmdDesc) + 2*((PEcInitCmdDesc)(p))->ecHead.len])
#define	EcInitCmdDescComment(p)	(PCHAR)&(((unsigned char*)(p))[sizeof(EcInitCmdDesc) + ((PEcInitCmdDesc)(p))->ecHead.len + \
	(((PEcInitCmdDesc)(p))->validate ? ((PEcInitCmdDesc)(p))->ecHead.len : 0) + \
	(((PEcInitCmdDesc)(p))->validateMask ? ((PEcInitCmdDesc)(p))->ecHead.len : 0)])

///////////////////////////////////////////////////////////////////////////////
#define	ECCANOPENCMDDESC_TYPE_AUTO			0
#define	ECCANOPENCMDDESC_TYPE_DD			1
#define	ECCANOPENCMDDESC_TYPE_USER			2


///////////////////////////////////////////////////////////////////////////////
// Mailbox
#define	ETHERCAT_MBOX_TYPE_ADS				1		// AMS/ADS header follows
#define	ETHERCAT_MBOX_TYPE_ETHERNET			2		// ETHERCAT_EOE_HEADER follows
#define	ETHERCAT_MBOX_TYPE_CANOPEN			3		// ETHERCAT_CANOPEN_HEADER follows
#define	ETHERCAT_MBOX_TYPE_FILEACCESS		4		// ETHERCAT_FOE_HEADER follows
#define	ETHERCAT_MBOX_TYPE_SOE				5		// ETHERCAT_SOE_HEADER follows

typedef struct TETHERCAT_MBOX_HEADER
{
	unsigned short	Length;							// following bytes
	unsigned short	Address;						// S->M: phys addr of destination; M->S: phys addr of source; 0 = master
	unsigned short	Channel			: 6;			// optional communication channels (default = 0)
	unsigned short	Priority		: 2;			// optional communication priority (default = 0)
	unsigned short	Type			: 4;			// TETHERCAT_MBOX_TYPE_xxx
	unsigned short	Reserved		: 3;			// 
	unsigned short	Unsupported		: 1;			// unsupported protocol detected
} ETHERCAT_MBOX_HEADER, *PETHERCAT_MBOX_HEADER;

#define	ETHERCAT_MBOX_HEADER_LEN	sizeof(ETHERCAT_MBOX_HEADER)

typedef struct TETHERCAT_MBOX_CMD
{
	ETYPE_EC_HEADER			EcHead;
	ETHERCAT_MBOX_HEADER	MBoxHead;
} ETHERCAT_MBOX_CMD, *PETHERCAT_MBOX_CMD;

#define	ETHERCAT_MBOX_CMD_LEN	sizeof(ETHERCAT_MBOX_CMD)


///////////////////////////////////////////////////////////////////////////////
// CoE (CANopen over EtherCAT)
#define	ETHERCAT_CANOPEN_TYPE_EMERGENCY	1		// 
#define	ETHERCAT_CANOPEN_TYPE_SDOREQ	2		// 
#define	ETHERCAT_CANOPEN_TYPE_SDORES	3		// 
#define	ETHERCAT_CANOPEN_TYPE_TXPDO		4		// 
#define	ETHERCAT_CANOPEN_TYPE_RXPDO		5		// 
#define	ETHERCAT_CANOPEN_TYPE_TXPDO_RTR	6		// Remote transmission request of TXPDO (master requested)
#define	ETHERCAT_CANOPEN_TYPE_RXPDO_RTR	7		// Remote transmission request of RXPDO (slave requested)
#define	ETHERCAT_CANOPEN_TYPE_SDOINFO	8		// 

typedef struct TETHERCAT_CANOPEN_HEADER
{
	unsigned short	Number			    : 9;		// e.g. PDO number
	unsigned short	Reserved			: 3;		// = 0
	unsigned short	Type				: 4;		// CANopen type
} ETHERCAT_CANOPEN_HEADER, *PETHERCAT_CANOPEN_HEADER;
#define	ETHERCAT_CANOPEN_HEADER_LEN	sizeof(ETHERCAT_CANOPEN_HEADER)


typedef struct TETHERCAT_SDO_HEADER
{
	union
	{
		struct
		{	// Initiate Download Request
			unsigned char	SizeInd		: 1;
			unsigned char	Expedited	: 1;
			unsigned char	Size		: 2;
			unsigned char	Complete	: 1;
			unsigned char	Ccs			: 3;	// = 1
		} Idq;
		struct
		{	// Initiate Download Response
			unsigned char	Reserved	: 5;
			unsigned char	Scs			: 3;	// = 3
		} Ids;
		struct
		{	// Download Segment Request
			unsigned char	LastSeg		: 1;
			unsigned char	Size		: 3;
			unsigned char	Toggle		: 1;
			unsigned char	Ccs			: 3;	// = 0
		} Dsq;
		struct
		{	// Download Segment Response
			unsigned char	Reserved	: 4;
			unsigned char	Toggle		: 1;
			unsigned char	Scs			: 3;	// = 1
		} Dss;
		struct
		{	// Initiate Upload Request
			unsigned char	Reserved	: 4;
			unsigned char	Complete	: 1;
			unsigned char	Ccs			: 3;	// = 2
		} Iuq;
		struct
		{	// Initiate Upload Response
			unsigned char	SizeInd		: 1;
			unsigned char	Expedited	: 1;
			unsigned char	Size		: 2;
			unsigned char	Reserved	: 1;
			unsigned char	Scs			: 3;	// = 2
		} Ius;
		struct
		{	// Upload Segment Request
			unsigned char	Reserved	: 4;
			unsigned char	Toggle		: 1;
			unsigned char	Ccs			: 3;	// = 3
		} Usq;
		struct
		{	// Upload Segment Response
			unsigned char	LastSeg		: 1;
			unsigned char	Size		: 3;
			unsigned char	Toggle		: 1;
			unsigned char	Scs			: 3;	// = 0
		} Uss;
		struct
		{	// Abort Transfer
			unsigned char	Reserved	: 5;
			unsigned char	Ccs			: 3;	// = 4
		} Abt;
		unsigned char		CS;								// 
	};
	unsigned short	Index;							// 
	unsigned char		SubIndex;						// 
	unsigned long		Data;								// 
} ETHERCAT_SDO_HEADER, *PETHERCAT_SDO_HEADER;

#define	ETHERCAT_SDO_HEADER_LEN	sizeof(ETHERCAT_SDO_HEADER)
#define	ETHERCAT_MIN_SDO_MBOX_LEN		(ETHERCAT_MBOX_HEADER_LEN + ETHERCAT_CANOPEN_HEADER_LEN + ETHERCAT_SDO_HEADER_LEN)

#define	SDO_CCS_DOWNLOAD_SEGMENT		0
#define	SDO_CCS_INITIATE_DOWNLOAD		1
#define	SDO_CCS_INITIATE_UPLOAD			2
#define	SDO_CCS_UPLOAD_SEGMENT			3
#define	SDO_CCS_ABORT_TRANSFER			4


#define	SDO_SCS_UPLOAD_SEGMENT			0
#define	SDO_SCS_DOWNLOAD_SEGMENT		1
#define	SDO_SCS_INITIATE_UPLOAD			2
#define	SDO_SCS_INITIATE_DOWNLOAD		3

#define	SDO_DOWNLOAD_SEGMENT_MAX_DATA	7

#define	SDO_ABORTCODE_TOGGLE			0x05030000	// Toggle bit not alternated.
#define	SDO_ABORTCODE_TIMEOUT			0x05040000	// SDO protocol timed out.
#define	SDO_ABORTCODE_CCS_SCS			0x05040001	// Client/server command specifier not valid or unknown.
#define	SDO_ABORTCODE_BLK_SIZE			0x05040002	// Invalid block size (block mode only).
#define	SDO_ABORTCODE_SEQNO				0x05040003	// Invalid sequence number (block mode only).
#define	SDO_ABORTCODE_CRC				0x05040004	// CRC error (block mode only).
#define	SDO_ABORTCODE_MEMORY			0x05040005	// Out of memory.
#define	SDO_ABORTCODE_ACCESS			0x06010000	// Unsupported access to an object.
#define	SDO_ABORTCODE_WRITEONLY			0x06010001	// Attempt to read a write only object.
#define	SDO_ABORTCODE_READONLY			0x06010002	// Attempt to write a read only object.
#define	SDO_ABORTCODE_INDEX				0x06020000	// Object does not exist in the object dictionary.
#define	SDO_ABORTCODE_PDO_MAP			0x06040041	// Object cannot be mapped to the PDO.
#define	SDO_ABORTCODE_PDO_LEN			0x06040042	// The number and length of the objects to be mapped would exceed PDO length.
#define	SDO_ABORTCODE_P_INCOMP			0x06040043	// General parameter incompatibility reason.
#define	SDO_ABORTCODE_I_INCOMP			0x06040047	// General internal incompatibility in the device.
#define	SDO_ABORTCODE_HARDWARE			0x06060000	// Access failed due to an hardware error.
#define	SDO_ABORTCODE_DATA_SIZE			0x06070010	// Data type does not match, length of service parameter does not match
#define	SDO_ABORTCODE_DATA_SIZE1		0x06070012	// Data type does not match, length of service parameter too high
#define	SDO_ABORTCODE_DATA_SIZE2		0x06070013	// Data type does not match, length of service parameter too low
#define	SDO_ABORTCODE_OFFSET			0x06090011	// Sub-index does not exist.
#define	SDO_ABORTCODE_DATA_RANGE		0x06090030	// Value range of parameter exceeded (only for write access).
#define	SDO_ABORTCODE_DATA_RANGE1		0x06090031	// Value of parameter written too high.
#define	SDO_ABORTCODE_DATA_RANGE2		0x06090032	// Value of parameter written too low.
#define	SDO_ABORTCODE_MINMAX			0x06090036	// Maximum value is less than minimum value.
#define	SDO_ABORTCODE_GENERAL			0x08000000	// general error
#define	SDO_ABORTCODE_TRANSFER			0x08000020	// Data cannot be transferred or stored to the application.
#define	SDO_ABORTCODE_TRANSFER1			0x08000021	// Data cannot be transferred or stored to the application because of local control.
#define	SDO_ABORTCODE_TRANSFER2			0x08000022	// Data cannot be transferred or stored to the application because of the present device state.
#define	SDO_ABORTCODE_DICTIONARY		0x08000023	// Object dictionary dynamic generation fails or no object dictionary is present (e.g. object dictionary is generated from file and generation fails because of an file error).


#define	SDO_SUB_IDX_SUBINDEX_CNT		0

#define	SDO_IDX_DEVICE_TYPE				0x1000
#define	SDO_IDX_ERROR					0x1001
#define	SDO_IDX_MANUFACTURER_NAME		0x1008
#define	SDO_IDX_MANUFACTURER_HW_VER		0x1009
#define	SDO_IDX_MANUFACTURER_SW_VER		0x100A
#define	SDO_IDX_IDENTITY_OBJECT			0x1018

#define	SDO_SIDX_IDENTITY_OBJECT_VENDORID	1
#define	SDO_SIDX_IDENTITY_OBJECT_PCODE		2
#define	SDO_SIDX_IDENTITY_OBJECT_REVNO		3
#define	SDO_SIDX_IDENTITY_OBJECT_SERNO		4

#define	SDO_IDX_ETHERCAT_ADDR			0x1100

#define	SDO_IDX_RXPDO1_PARA				0x1400
#define	SDO_SIDX_RXPDXX_PARA_EXCLUDE		1

#define	SDO_IDX_RXPDO2_PARA				0x1401
// ...
#define	SDO_IDX_RXPDO512_PARA			0x15FF

#define	SDO_IDX_RXPDO1_MAPPING			0x1600
#define	SDO_IDX_RXPDO2_MAPPING			0x1601
// ...
#define	SDO_IDX_RXPDO512_MAPPING		0x17FF

#define	SDO_IDX_TXPDO1_PARA				0x1800
#define	SDO_IDX_TXPDO2_PARA				0x1801
// ...
#define	SDO_IDX_TXPDO512_PARA			0x19FF

#define	SDO_IDX_TXPDO1_MAPPING			0x1A00
#define	SDO_IDX_TXPDO2_MAPPING			0x1A01
// ...
#define	SDO_IDX_TXPDO512_MAPPING		0x1BFF

#define	SDO_IDX_SYNCMAN_TYPE			0x1C00
#define	SDO_IDX_SYNCMAN0_PDOASSIGN		0x1C10
#define	SDO_IDX_SYNCMAN1_PDOASSIGN		0x1C11
#define	SDO_IDX_SYNCMAN2_PDOASSIGN		0x1C12
#define	SDO_IDX_SYNCMAN3_PDOASSIGN		0x1C13
// ...
#define	SDO_IDX_SYNCMAN31_PDOASSIGN	0x1C2F

#define	SDO_IDX_ECAT_MEMORY_0000_00FF	0x1D00
#define	SDO_IDX_ECAT_MEMORY_0100_01FF	0x1D01
// ...
#define	SDO_IDX_ECAT_MEMORY_FF00_FFFF	0x1DFF

// CoE SDO Information
#define	ECAT_COE_INFO_OPCODE_LIST_Q		1
#define	ECAT_COE_INFO_OPCODE_LIST_S		2
#define	ECAT_COE_INFO_OPCODE_OBJ_Q		3
#define	ECAT_COE_INFO_OPCODE_OBJ_S		4
#define	ECAT_COE_INFO_OPCODE_ENTRY_Q	5
#define	ECAT_COE_INFO_OPCODE_ENTRY_S	6
#define	ECAT_COE_INFO_OPCODE_ERROR_S	7

#define	ECAT_COE_INFO_LIST_TYPE_LENGTH		0
#define	ECAT_COE_INFO_LIST_TYPE_ALL			1
#define	ECAT_COE_INFO_LIST_TYPE_RXPDOMAP	2
#define	ECAT_COE_INFO_LIST_TYPE_TXPDOMAP	3
#define	ECAT_COE_INFO_LIST_TYPE_BACKUP		4

#define	ECAT_COE_INFO_OBJCODE_NULL			0
#define	ECAT_COE_INFO_OBJCODE_DOMAIN		2
#define	ECAT_COE_INFO_OBJCODE_DEFTYPE		5
#define	ECAT_COE_INFO_OBJCODE_DEFSTRUCT		6
#define	ECAT_COE_INFO_OBJCODE_VAR			7
#define	ECAT_COE_INFO_OBJCODE_ARRAY			8
#define	ECAT_COE_INFO_OBJCODE_RECORD		9

#define	ECAT_COE_INFO_OBJCAT_OPTIONAL		0
#define	ECAT_COE_INFO_OBJCAT_MANDATORY		1

#define	ECAT_COE_INFO_OBJACCESS_RO			0x07
#define	ECAT_COE_INFO_OBJACCESS_RW			0x3f

typedef struct TETHERCAT_SDO_INFO_LIST
{
	unsigned short	ListType;				// == SDO_INFO_LIST_TYPE_XXX
	struct
	{
		unsigned short	Index[1];		
	} Res;
} ETHERCAT_SDO_INFO_LIST, *PETHERCAT_SDO_INFO_LIST;

typedef struct TETHERCAT_SDO_INFO_OBJ
{
	unsigned short	Index;
	struct
	{
		unsigned short		DataType;				// refer to data type index
		unsigned char		MaxSubIndex;			// max subIndex
		unsigned char		ObjCode			: 4;	// defined in DS 301 (Table 37)
		unsigned char		ObjCategory		: 1;	// 0=optional, 1=mandatory
		unsigned char		Reserved		: 3;	// == 0
		char		Name[1];				// rest of mailbox data
	} Res;
} ETHERCAT_SDO_INFO_OBJ, *PETHERCAT_SDO_INFO_OBJ;

typedef struct TETHERCAT_SDO_INFO_ENTRY
{
	unsigned short	Index;
	unsigned char		SubIdx;	
	unsigned char		ValueInfo;				// bit0 = ObjAccess, bit1 = ObjCategory, bit2 = PdoMapping, bit3 = UnitType
										// bit4 = DefaultValue, bit5 = MinValue, bit6 = MaxValue
	struct
	{
		unsigned short	DataType;				// refer to data type index
		unsigned short	BitLen;
		unsigned short	ObjAccess		: 6;	// bit0 = read; bit1 = write; bit2 = const. bit3 = �PRE-OP�; bit4 = �SAFE-OP�; bit5 = �OP�.
		unsigned short	RxPdoMapping	: 1;	// 
		unsigned short	TxPdoMapping	: 1;	// 
		unsigned short	Reserved		: 8;	// for future use
	} Res;
} ETHERCAT_SDO_INFO_ENTRY, *PETHERCAT_SDO_INFO_ENTRY;

typedef struct TETHERCAT_SDO_INFO_ERROR
{
	unsigned long		ErrorCode;
	char		ErrorText[1];				// rest of mailbox data
} ETHERCAT_SDO_INFO_ERROR, *PETHERCAT_SDO_INFO_ERROR;

typedef struct TETHERCAT_SDO_INFO_HEADER
{
	unsigned char		OpCode			: 7;			// == SDO_INFO_TYPE_XXX
	unsigned char		InComplete		: 1;			// 

	unsigned char		Reserved;						// == 0

	unsigned short		FragmentsLeft;					// 

	union
	{
		ETHERCAT_SDO_INFO_LIST	List;
		ETHERCAT_SDO_INFO_OBJ	Obj;
		ETHERCAT_SDO_INFO_ENTRY Entry;
		ETHERCAT_SDO_INFO_ERROR Error;
		unsigned char					Data[1];
	};
} ETHERCAT_SDO_INFO_HEADER, *PETHERCAT_SDO_INFO_HEADER;




//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define	ETHERCAT_SDO_INFO_LISTREQ_LEN		offsetof(ETHERCAT_SDO_INFO_HEADER, List.Res)
#define	ETHERCAT_SDO_INFO_OBJREQ_LEN		offsetof(ETHERCAT_SDO_INFO_HEADER, Obj.Res)
#define	ETHERCAT_SDO_INFO_ENTRYREQ_LEN		offsetof(ETHERCAT_SDO_INFO_HEADER, Entry.Res)

typedef struct TETHERCAT_EMERGENCY_HEADER
{
	unsigned short		ErrorCode;
	unsigned char		ErrorRegister;
	unsigned char		Data[5];
} ETHERCAT_EMERGENCY_HEADER, *PETHERCAT_EMERGENCY_HEADER;



///////////////////////////////////////////////////////////////////////////////
// FoE (File Access over EtherCAT)
#define	ECAT_FOE_OPCODE_RRQ					1
#define	ECAT_FOE_OPCODE_WRQ					2
#define	ECAT_FOE_OPCODE_DATA				3
#define	ECAT_FOE_OPCODE_ACK					4
#define	ECAT_FOE_OPCODE_ERR					5
#define	ECAT_FOE_OPCODE_BUSY				6

#define	ECAT_FOE_ERRCODE_NOTDEFINED			0
#define	ECAT_FOE_ERRCODE_NOTFOUND			1
#define	ECAT_FOE_ERRCODE_ACCESS				2
#define	ECAT_FOE_ERRCODE_DISKFULL			3
#define	ECAT_FOE_ERRCODE_ILLEAGAL			4
#define	ECAT_FOE_ERRCODE_PACKENO			5
#define	ECAT_FOE_ERRCODE_EXISTS				6
#define	ECAT_FOE_ERRCODE_NOUSER				7
#define	ECAT_FOE_ERRCODE_BOOTSTRAPONLY		8
#define	ECAT_FOE_ERRCODE_NOTINBOOTSTRAP		9
#define	ECAT_FOE_ERRCODE_INVALIDPASSWORD	10

typedef struct TETHERCAT_FOE_HEADER
{
	unsigned char		OpCode;			// = 1 (RRQ), = 2 (WRQ), = 3 (DATA), = 4 (ACK), = 5 (ERR), = 6 (BUSY)
	unsigned char		Reserved1;		// = 0
	union
	{
		unsigned long		Password;	// (RRQ, WRQ)       = 0 if unknown
		unsigned long		PacketNo;	// (DATA, ACK)
		unsigned long		ErrorCode;	// (ERR)
		struct
		{
			unsigned short	Done;		// (BUSY)
			unsigned short	Entire;		// (BUSY)
		};
	};
} ETHERCAT_FOE_HEADER, *PETHERCAT_FOE_HEADER;

#define	ETHERCAT_FOE_HEADER_LEN	sizeof(ETHERCAT_FOE_HEADER)

typedef struct TETHERCAT_FOE_BUSY_INFO
{
	unsigned short	Done;
	unsigned short	Entire;
	char		Comment[32];
} ETHERCAT_FOE_BUSY_INFO, *PETHERCAT_FOE_BUSY_INFO;

#define	ETHERCAT_FOE_BUSY_INFO_LEN		sizeof(ETHERCAT_FOE_BUSY_INFO)

///////////////////////////////////////////////////////////////////////////////
// SoE (Servo Drive over EtherCAT)
#define	ECAT_SOE_OPCODE_RRQ		1
#define	ECAT_SOE_OPCODE_RRS		2
#define	ECAT_SOE_OPCODE_WRQ		3
#define	ECAT_SOE_OPCODE_WRS		4
#define	ECAT_SOE_OPCODE_NFC		5
#define	ECAT_SOE_OPCODE_EMGCY	6

typedef struct TETHERCAT_SOE_ELEMENT
{
	union
	{
		struct
		{
			unsigned char		DataState	: 1;
			unsigned char		Name		: 1;
			unsigned char		Attribute	: 1;
			unsigned char		Unit		: 1;
			unsigned char		Min			: 1;
			unsigned char		Max			: 1;
			unsigned char		Value		: 1;
			unsigned char		Default		: 1;
		};
		unsigned char	Elements;
	};
} ETHERCAT_SOE_ELEMENT;

typedef struct TETHERCAT_SOE_HEADER
{
	unsigned char	OpCode		: 3;	// 0 = unused, 1 = readReq, 2 = readRes, 3 = writeReq, 4 = writeRes
								// 5 = notification (command changed notification)
	unsigned char	InComplete	: 1;	// more follows
	unsigned char	Error		: 1;	// an error word follows
	unsigned char	DriveNo		: 3;	// drive number

	union
	{
		struct
		{
			unsigned char		DataState	: 1;
			unsigned char		Name		: 1;
			unsigned char		Attribute	: 1;
			unsigned char		Unit		: 1;
			unsigned char		Min			: 1;
			unsigned char		Max			: 1;
			unsigned char		Value		: 1;
			unsigned char		Default		: 1;
		};
		unsigned char	Elements;
	};

	union 
	{
		unsigned short	IDN	;			// SoE IDN					if (InComplete==0)
		unsigned short	FragmentsLeft;	// Pending fragments			if (InComplete==1) 
	};
} ETHERCAT_SOE_HEADER, *PETHERCAT_SOE_HEADER;

#define	ETHERCAT_SOE_HEADER_LEN	sizeof(ETHERCAT_SOE_HEADER)

typedef struct TETHERCAT_SOE_MAILBOX_IOFFS
{
	unsigned short	IDN;
	union
	{
		struct
		{
			unsigned char		DataState	: 1;
			unsigned char		Name		: 1;
			unsigned char		Attribute	: 1;
			unsigned char		Unit		: 1;
			unsigned char		Min			: 1;
			unsigned char		Max			: 1;
			unsigned char		Value		: 1;
			unsigned char		Default		: 1;
		};
		unsigned char	Elements;
	};
	unsigned char		DriveNo		: 3;
	unsigned char		Reserved2	: 4;
	unsigned char		Command		: 1;
} ETHERCAT_SOE_MAILBOX_IOFFS, *PETHERCAT_SOE_MAILBOX_IOFFS;

///////////////////////////////////////////////////////////////////////////////
typedef struct TEcMailboxCmdDesc
{
	unsigned short					transition;
	unsigned short					protocol;	
	unsigned long					dataLen;
	unsigned short					cmtLen;	    // (excl. \0)
	unsigned short					timeout;	// in ms
	unsigned short					retries;	
	unsigned short					reserved1;	
	unsigned long					reserved2[4];
	union
	{
		struct
		{
			ETHERCAT_SDO_HEADER	sdo;
			unsigned char						data[1];		// data[dataLen-sizeof(sdo)];
		} coe;
		struct
		{
			ETHERCAT_SOE_HEADER	head;
			unsigned long						attribute;
			unsigned char						data[1];		// data[dataLen-sizeof(head)];				
		} soe;
		struct
		{
			unsigned short					nameSize;
			unsigned short					reserved;
			unsigned long					password;
			char					name[1];	 		// no \0
		} foe;
		unsigned char						data[1];			// data[dataLen];
	};
} EcMailboxCmdDesc, *PEcMailboxCmdDesc;


#define	SIZEOF_EcMailboxCmdDesc(p)	(offsetof(EcMailboxCmdDesc, data) + ((PEcMailboxCmdDesc)(p))->dataLen + ((PEcMailboxCmdDesc)(p))->cmtLen + 1)
#define	NEXT_EcMailboxCmdDesc(p)	(PEcMailboxCmdDesc)&(((unsigned char*)(p))[SIZEOF_EcMailboxCmdDesc(p)])
#define	EcMailboxCmdDescComment(p)	(PCHAR)&(((unsigned char*)(p))[offsetof(EcMailboxCmdDesc, data) + ((PEcMailboxCmdDesc)(p))->dataLen])
#define	SIZEOF_EcMailboxCmdDescCoeData(p)	(((PEcMailboxCmdDesc)(p))->dataLen-sizeof(ETHERCAT_SDO_HEADER))
#define	SIZEOF_EcMailboxCmdDescSoeData(p)	(((PEcMailboxCmdDesc)(p))->dataLen-sizeof(ETHERCAT_SOE_HEADER)-sizeof(unsigned long))
 



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// EcServices.h


#define	ESC_REG_ESC_TYPE				0x0000
#define	ESC_REG_ESC_REVISION			0x0001
#define	ESC_REG_ESC_BUILD				0x0002

#define	ESC_REG_FEATURES				0x0008
#define	ESC_FEATURE_DC_AVAILABLE		0x0004
#define	ESC_FEATURE_DC64_AVAILABLE		0x0008
	
#define	ESC_REG_ESC_STATUS				0x0110

#define	ESC_STATUS_LOOP_A_MASK			0x0300
#define	ESC_STATUS_LOOP_A_CLOSE			0x0100
#define	ESC_STATUS_LOOP_A_LINK			0x0200
#define	ESC_STATUS_LOOP_B_MASK			0x0C00
#define	ESC_STATUS_LOOP_B_CLOSE			0x0400
#define	ESC_STATUS_LOOP_B_LINK			0x0800
#define	ESC_STATUS_LOOP_C_MASK			0x3000
#define	ESC_STATUS_LOOP_C_CLOSE			0x1000
#define	ESC_STATUS_LOOP_C_LINK			0x2000
#define	ESC_STATUS_LOOP_D_MASK			0xC000
#define	ESC_STATUS_LOOP_D_CLOSE			0x4000
#define	ESC_STATUS_LOOP_D_LINK			0x8000

#define	ESC_REG_DC_SOF_LATCH_A			0x0900
#define	ESC_REG_DC_SOF_LATCH_B			0x0904
#define	ESC_REG_DC_SOF_LATCH_C			0x0908
#define	ESC_REG_DC_SOF_LATCH_D			0x090c

#define	ESC_REG_DC_SYSTIME				0x0910
#define	ESC_REG_DC_SOF_LATCH_A_64		0x0918
#define	ESC_REG_DC_SYSTIME_OFFS			0x0920
#define	ESC_REG_DC_SYSTIME_DELAY		0x0928
#define	ESC_REG_DC_CONTROL_ERROR		0x092c
#define	ESC_REG_DC_SPEED_START			0x0930
#define	ESC_REG_DC_SPEED_DIFF			0x0932
#define	ESC_REG_DC_FILTER_EXP			0x0934
#define	ESC_REG_DC_SOF_LATCH_FRAME		0x0936

#define	DEVICE_STATE_MASK					0x000F
#define	DEVICE_STATE_INIT					0x0001
#define	DEVICE_STATE_PREOP					0x0002
#define	DEVICE_STATE_BOOTSTRAP				0x0003
#define	DEVICE_STATE_SAFEOP					0x0004
#define	DEVICE_STATE_OP						0x0008
#define	DEVICE_STATE_ERROR					0x0010

const	int64_t	ECAT_DCSTARTIME_IN_SYSTEMTIME	= 0x01bf53eb256d4000;

#define	ECAT_DCTIME_TO_SYSTIME_FACTOR		100
#define	ECAT_SYSTIME_TO_DCTIME_64(s)		(((s)-ECAT_DCSTARTIME_IN_SYSTEMTIME)*ECAT_DCTIME_TO_SYSTIME_FACTOR)
#define	ECAT_SYSTIME_TO_DCTIME_32(s)		((unsigned long)ECAT_SYSTIME_TO_DCTIME_64(s))
#define	ECAT_DCTIME_TO_SYSTIME_64(d)		(((d)/ECAT_DCTIME_TO_SYSTIME_FACTOR)+ECAT_DCSTARTIME_IN_SYSTEMTIME)
#define	ECAT_DCTIME_TO_SYSTIME_32(d)		((unsigned long)ECAT_DCTIME_TO_SYSTIME_64(d))
#define	ECAT_SYSBASE_TO_DCBASE(s)			((s)*ECAT_DCTIME_TO_SYSTIME_FACTOR)
#define	ECAT_DCBASE_TO_SYSBASE(d)			((d)/ECAT_DCTIME_TO_SYSTIME_FACTOR)

typedef enum EECAT_DC_MODE
{
	ECAT_DC_MODE_NONE					= 0,
	ECAT_DC_MODE_TOPOLOGY				= 1,
	ECAT_DC_MODE_SLAVE					= 2,
	ECAT_DC_MODE_MASTER					= 3,
} ECAT_DC_MODE;

#define	ECAT_DEVICE_NAMESIZE				31		


typedef struct TEcMasterDesc
{
	unsigned char					addr[8];
	ETHERNET_ADDRESS		macAddr;
	unsigned short					maxSlaves;
	char					szName[ECAT_DEVICE_NAMESIZE+1];
	unsigned short					id;
	struct
	{
		unsigned long						suppressMBoxPolling	: 1;
		unsigned long						reserved3			: 1;
		unsigned long						dcSyncMode			: 4;
		unsigned long						ioOpenStateInit		: 1;
		unsigned long						logTopologyChanges	: 1;
		unsigned long						disableContMeasuring	: 1;
		unsigned long						disableContCrcCheck	: 1;
		unsigned long						dynamicTopology		: 1;
		unsigned long						waitForStartIo		: 1;
		unsigned long						forceReInit			: 1;
	};
	unsigned long					logAddressMBoxStates;	// start address of log. area of sync man states
	unsigned short					sizeAddressMBoxStates;	// size of log. area of sync man states
	unsigned short					cycOnlyBufferCnt;	
	unsigned short					initcmdCnt;
	unsigned short					initcmdLen;
	unsigned long					dcCycleTime;
	long							dcShiftTime;
	ETYPE_VLAN_HEADER				vlanInfo;
	unsigned long					dcSyncWindow;
	unsigned long					reserved2;
} EcMasterDesc, *PEcMasterDesc;

#define	SIZEOF_EcMasterDesc(p)			(sizeof(EcMasterDesc)+((PEcMasterDesc)p)->initcmdLen)

typedef struct TEcSlaveTopologyDesc
{
	unsigned short		    physAddr[4];		// self, portA, portB, portC
	unsigned long			portDelay[2];		// portB, portC
} EcSlaveTopologyDesc, *PEcSlaveTopologyDesc;

typedef enum TECAT_SLAVE_TYPE
{
	ECAT_SLAVE_TYPE_NONE			= 0,
	ECAT_SLAVE_TYPE_SIMPLE			= 1,
	ECAT_SLAVE_TYPE_MAILBOX			= 2,
	ECAT_SLAVE_TYPE_ROUTER			= 3,
	ECAT_SLAVE_TYPE_AX2000B100		= 4,
	ECAT_SLAVE_TYPE_EL67XX			= 5,
} ECAT_SLAVE_TYPE;


#define	ECAT_ESC_PORTS_MAX		4
static	int	ECAT_PORT_ORDER[ECAT_ESC_PORTS_MAX] = {0, 3, 1, 2};

typedef struct TEcMasterCreateSlave
{
	unsigned short						autoIncAddr;
	unsigned short						physAddr;
	unsigned short						type;
	struct
	{
		unsigned short						reserved0				: 1;
		unsigned short						mboxOutShortSend		: 1;
		unsigned short						cycleMBoxPolling		: 1;
		unsigned short						stateMBoxPolling		: 1;
		unsigned short						referenceClock			: 1;
		unsigned short						forceEndOfBranch		: 1;
		unsigned short						eoeSupport				: 1;
		unsigned short						coeSupport				: 1;
		unsigned short						foeSupport				: 1;
		unsigned short						soeSupport				: 1;
	};
	unsigned short						slaveAddressMBoxState;			// bit offset in logical area
	unsigned short						cycleMBoxPollingTime;			// cycle time for MBox polling
	char								szName[ECAT_DEVICE_NAMESIZE+1];	
	unsigned long						virtualIpAddr;
	ETHERNET_ADDRESS					virtualMacAddr;
	unsigned short						mboxOutStart;
	unsigned short						mboxOutLen;
	unsigned short						mboxInStart;
	unsigned short						mboxInLen;
	unsigned short						initcmdCnt;
	unsigned short						initcmdLen;
	unsigned short						mboxCmdCnt;
	unsigned short						mboxCmdLen;
	unsigned long						pdataOutStart;
	unsigned long						pdataInStart;
	unsigned short						pdataOutLen;
	unsigned short						pdataInLen;
	unsigned short						reserved2;
	unsigned short						bootOutStart;
	unsigned short						bootOutLen;
	unsigned short						bootInStart;
	unsigned short						bootInLen;
	unsigned short						prevPhysAddr;	
	unsigned short						prevPort;
} EcMasterCreateSlave, *PEcMasterCreateSlave;

#define	SIZEOF_EcMasterCreateSlave(p)			(sizeof(EcMasterCreateSlave)+((PEcMasterCreateSlave)p)->initcmdLen+((PEcMasterCreateSlave)p)->mboxCmdLen)



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// EcInterface.h

#ifndef	VG_IN
#define	VG_IN	0
#define	VG_OUT	1
#endif

#define	ETHERNETRTMP_STATE_LINKERROR		0x0001
#define	ETHERNETRTMP_STATE_LOCKLINKERROR	0x0002
#define	ETHERNETRTMP_STATE_ALLOCFRAME		0x0010
#define	ETHERNETRTMP_STATE_WATCHDOG			0x0020
#define	ETHERNETRTMP_STATE_NOMINIPORT		0x0040
#define	ETHERNETRTMP_STATE_RESETACTIVE		0x0080
#define	ETHERNETRTMP_STATE_CYC_WCNTOUT_ERR	0x0100
#define	ETHERNETRTMP_STATE_CYC_WCNTIN_ERR	0x0200

#define MAILBOXCMD_CANOPEN_BEGIN			0xF300		// begin of reserved CANopen area
#define MAILBOXCMD_CANOPEN_SDO				0xF302		
#define MAILBOXCMD_CANOPEN_SDO_INFO_LIST	0xF3FC		// listType = index
#define MAILBOXCMD_CANOPEN_SDO_INFO_OBJ		0xF3FD		
#define MAILBOXCMD_CANOPEN_SDO_INFO_ENTRY	0xF3FE		
#define MAILBOXCMD_CANOPEN_END				0xF3FF		// end of reserved CANopen area

#define MAILBOXCMD_ECAT_FOE_BEGIN			0xF400	// File Access over EtherCAT
#define MAILBOXCMD_ECAT_FOE_FOPENREAD		0xF401	// 
#define MAILBOXCMD_ECAT_FOE_FOPENWRITE		0xF402	// 
#define MAILBOXCMD_ECAT_FOE_FCLOSE			0xF403	// 
#define MAILBOXCMD_ECAT_FOE_FREAD			0xF404	// 
#define MAILBOXCMD_ECAT_FOE_FWRITE			0xF405	// 
#define MAILBOXCMD_ECAT_FOE_PROGRESSINFO	0xF406	// 
#define MAILBOXCMD_ECAT_FOE_END				0xF41F	// File Access over EtherCAT

#define MAILBOXCMD_ECAT_SOE					0xF420	// Servo Drive over EtherCAT: IDN = LOWORD(iOffs), ELEMENT = LOBYTE(HIWORD(iOffs))


#define MAILBOXIOFFS_ECAT_SOE_ELEMENT_MASK		0x00FF0000	// element mask of soe idn
#define MAILBOXIOFFS_ECAT_SOE_DATASTATE			0x00010000	// element of soe idn
#define MAILBOXIOFFS_ECAT_SOE_NAME				0x00020000	// element of soe idn
#define MAILBOXIOFFS_ECAT_SOE_ATTRIBUTE			0x00040000	// element of soe idn
#define MAILBOXIOFFS_ECAT_SOE_UNIT				0x00080000	// element of soe idn
#define MAILBOXIOFFS_ECAT_SOE_MIN				0x00100000	// element of soe idn
#define MAILBOXIOFFS_ECAT_SOE_MAX				0x00200000	// element of soe idn
#define MAILBOXIOFFS_ECAT_SOE_VALUE				0x00400000	// element of soe idn
#define MAILBOXIOFFS_ECAT_SOE_DEFAULT			0x00800000	// element of soe idn
#define MAILBOXIOFFS_ECAT_SOE_DRIVENO_MASK		0x07000000	// element mask of drive no
#define MAILBOXIOFFS_ECAT_SOE_COMMAND			0x08000000	// command execution requested

typedef enum EC_MAILBOX_CMD
{	
	EC_MAILBOX_CMD_UPLOAD					 = 1,
	EC_MAILBOX_CMD_DOWNLOAD					 = 2,	
	EC_MAILBOX_CMD_READWRITE				 = 3,	
}EC_MAILBOX_CMD;


typedef struct TEcMailboxAddr
{
	unsigned short	id;
	unsigned short	port;
} EcMailboxAddr, *PEcMailboxAddr;

typedef struct TEcMailboxCmd
{
	EcMailboxAddr addr;
	unsigned long		invokeId;		
	unsigned long		result;	
	unsigned long		length; // number of bytes to read or to write
	unsigned long		cmdId;	
	unsigned short		type;   // SDO_DOWNLOAD or SDO_UPLOAD 		
	union
	{
		unsigned long indexOffset;
		struct
		{						
			unsigned char		subIndex;	
			union
			{
				unsigned char		valueInfo;			
				unsigned char		complete;
			};
			unsigned short	index;
		};
	};
} EcMailboxCmd, *PEcMailboxCmd;



typedef struct TEcMailboxCmdRW 
{
	EcMailboxAddr addr;
	unsigned long		invokeId;		
	unsigned long		result;	
	unsigned long		length; // number of bytes to read or to write
	unsigned long		cmdId;	
	unsigned short	type;   // SDO_DOWNLOAD or SDO_UPLOAD 		
	union
	{
		unsigned long indexOffset;
		struct
		{						
			unsigned char		subIndex;	
			union
			{
				unsigned char		valueInfo;			
				unsigned char		complete;
			};
			unsigned short	index;
		};
	};

	unsigned long writeLength; 	
} EcMailboxCmdRW, *PEcMailboxCmdRW;





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// EcDevice


typedef struct TCANopenCmd
{
	ETHERCAT_CANOPEN_HEADER	head;					// head for the string command
} CANopenCmd, *PCANopenCmd;

//Andrea memoria
//#define	ETHERNETRTMP_MAXE88A4FIFO				1000
//#define	ETHERNETRTMP_MAXECATFRAMES				500
//#define	ETHERNETRTMP_MAXECUSERFIFO				1000
#define	ETHERNETRTMP_MAXE88A4FIFO				100
#define	ETHERNETRTMP_MAXECATFRAMES				50
#define	ETHERNETRTMP_MAXECUSERFIFO				100

#define	SetResetMask(v, m, b)					if (b) {(v) |= (m);} else {(v) &= ~(m);}

typedef struct TEcCmdImage
{
	union
	{
		struct
		{	// state
			unsigned short	bDropped		: 1;
			unsigned short	bNotReceived	: 1;
		};
		struct
		{	// ctrl
			unsigned short	bDisable	: 1;
		};
		unsigned short	ctrlState;
	};
	union
	{
		struct
		{	
			unsigned short	adp;	
			unsigned short	ado;	
		};
		unsigned long		laddr;
	};
	unsigned short	irq;	
	unsigned short	dataCnt;	// data or if no data in image: cnt
} EcCmdImage, *PEcCmdImage;

typedef struct TEcUserFrameInfo
{
	unsigned long				invokeId;
	unsigned long				cbLength;
	unsigned long				ecCmdHash;
	unsigned long				result;
	unsigned short				received;	
	unsigned short				cmdCount;
	unsigned short				timeout;
	unsigned short				timeFailed;
	unsigned char				indexBak;
} EcUserFrameInfo, *PEcUserFrameInfo;


typedef enum _EC_MEDIA_STATE 
{    	 
	EcMediaStateConnected,
    EcMediaStateDisconnected,
	EcMediaStateUndefined
} EC_MEDIA_STATE, *PEC_MEDIA_STATE;


#define RTMP_WATCHDOG_CYCLES	10

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// EcMaster




#define	MAX_RETRIES		3
#define	SYNC_REPEAT		5
#define	TIMEOUT_RELOAD	200 
 

#define GetListHeadEntry(ListHead)  ((ListHead)->Flink)
#define GetListTailEntry(ListHead)  ((ListHead)->Blink)


///////////////////////////////////////////////////////////////////////////////
#define	ETYPE_EC_CMD_FIT_IN_FRAME(p, n)		\
	(((PETYPE_88A4_HEADER)p)->Length + n < ETHERNET_MAX_FRAME_LEN-ETHERNET_88A4_FRAME_LEN )
#define	ETYPE_EC_CMD_CNT_FIT_IN_FRAME(n)		\
	(sizeof(ETHERNET_88A4_MAX_HEADER)-sizeof(ETYPE_88A4_HEADER))/(ETYPE_EC_OVERHEAD+(n))
#define	ETYPE_EC_CMD_FIT_IN_MAX_FRAME(m, p, n)		\
	(((PETYPE_88A4_HEADER)p)->Length + n < (m)-ETHERNET_88A4_FRAME_LEN )
#define	ETYPE_EC_CMD_CNT_FIT_IN_MAX_FRAME(m, n)		\
	((m)-ETHERNET_FRAME_LEN-ETYPE_88A4_HEADER_LEN)/(ETYPE_EC_OVERHEAD+(n))

#define ECAT_INITCMD_DUMMY_VALUE		0xffff
#define ECAT_DC_READ_TIMING_CNT_START		40



#define ECAT_DC_READ_TIMING_CNT_OP			1000


//Andrea memoria
//#define	MAX_SLAVECMD	50
//#define	MAX_SLAVEFRM	40
//#define	MAX_SLAVECMD	6
//#define	MAX_SLAVEFRM	5
#define	MAX_SLAVECMD	8
#define	MAX_SLAVEFRM	3

#define	MBOX_SLAVE_ONTIMER_DIV		5



///////////////////////////////////////////////////////////////////////////////
typedef struct	TEcCycImage
{
	union
	{
		struct
		{	// state
			unsigned short	bDropped	: 1;
			unsigned short	bError		: 1;
		};
		struct
		{	// ctrl
			unsigned short	bDisable	: 1;
		};
		unsigned short	frameCtrlState;
	};
	unsigned short	wdInfo;
} EcCycImage, *PEcCycImage;


#define	ETHERNETRTMP_MAXECFRAMES	10
#define	EcCycCmdInfo(p, n)			(&((PEcCmdInfo)ENDOF((PEcCycInfo)p))[n])

///////////////////////////////////////////////////////////////////////////////
typedef enum EEC_MASTER_STATE
{
	EC_MASTER_STATE_NONE						= 0x0000,

	EC_MASTER_STATE_INIT						= 0x0100,	

	EC_MASTER_STATE_DCLOCKS,

	EC_MASTER_STATE_WAIT_SLAVE_I_P,

	EC_MASTER_STATE_PREOP						= 0x0200,

	EC_MASTER_STATE_WAIT_SLAVE_P_I,
	EC_MASTER_STATE_WAIT_SLAVE_P_S,

	EC_MASTER_STATE_SAFEOP						= 0x0300,

	EC_MASTER_STATE_WAIT_SLAVE_S_I,
	EC_MASTER_STATE_WAIT_SLAVE_S_P,
	EC_MASTER_STATE_WAIT_SLAVE_S_O,

	EC_MASTER_STATE_OP							= 0x0400,

	EC_MASTER_STATE_WAIT_SLAVE_O_I,
	EC_MASTER_STATE_WAIT_SLAVE_O_P,
	EC_MASTER_STATE_WAIT_SLAVE_O_S,

} EC_MASTER_STATE;

#define MAILBOX_SDO_COMPLETE 1

typedef struct
{
	PETHERNET_FRAME	pFrame;
	unsigned long			nFrame;	
} EcAcycFrameInfo, *PEcAcycFrameInfo;


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// EcNpfDevice

#define  DEFAULT_ADAPTER_NAMELIST					8192
#define  MAX_NUM_ADAPTER							7
#define	 MAX_NPFPACKETS								100


typedef struct TEcAdapterInfo
{
	char szName[256];
	char szDescr[256];
}EcAdapterInfo, *pEcAdapterInfo;


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// EcSlave


#define	INITCMD_INACTIVE	0xffff


typedef	enum EECSI_DC
{	
	ECSI_DC_READ_FEATURES = 0x00020000,
	ECSI_DC_READ_LINK_STATUS,
	ECSI_DC_READ_TIMING,	
	ECSI_DC_WRITE_CLOCK_DELAY,
	ECSI_DC_WRITE_SYSTIME_OFFS,
	ECSI_DC_WRITE_SPEED_START,
	ECSI_DC_WRITE_CYCLE_START,
	ECSI_DC_WRITE_CYCLE_TIME,
	ECSI_DC_CHECK_BUILD_ARMW,
	ECSI_DC_WRITE_IRQ_START,	
} ECSI_DC;

#define	EC_MAX_PORTS				4
#define	EC_A						0
#define	EC_B						1
#define	EC_C						2
#define	EC_D						3

#define	EC_AD						3

#define	EC_DB						1
#define	EC_AB						1

#define	EC_BC						2
#define	EC_DC						2
#define	EC_AC						2

#define	EC_PORT_A_INUSE(p)	((p&ESC_STATUS_LOOP_A_MASK)==ESC_STATUS_LOOP_A_LINK)
#define	EC_PORT_B_INUSE(p)	((p&ESC_STATUS_LOOP_B_MASK)==ESC_STATUS_LOOP_B_LINK)
#define	EC_PORT_C_INUSE(p)	((p&ESC_STATUS_LOOP_C_MASK)==ESC_STATUS_LOOP_C_LINK)
#define	EC_PORT_D_INUSE(p)	((p&ESC_STATUS_LOOP_D_MASK)==ESC_STATUS_LOOP_D_LINK)
///////////////////////////////////////////////////////////////////////////////
typedef	enum EECSI
{
	ECSI_UNKNOWN,
	ECSI_MBOX_SEND				= 0x00010000,
	ECSI_MBOX_RECV,	
} ECSI;


///////////////////////////////////////////////////////////////////////////////
typedef struct TECMBSLAVE_COE_CMD_INFO
{
	PEcMailboxCmd	pCmd;
	PEcMailboxCmd	pRet;
	unsigned short			timeout;
} ECMBSLAVE_COE_CMD_INFO, *PECMBSLAVE_COE_CMD_INFO;


///////////////////////////////////////////////////////////////////////////////
typedef enum TECMBSLAVE_SOE_CMD_STATE
{
	ECMBSLAVE_SOE_CMD_STATE_3_WRITTEN,
	ECMBSLAVE_SOE_CMD_STATE_0_WRITTEN,
} ECMBSLAVE_SOE_CMD_STATE;

  
#define	ECMAILBOX_DEFAULT_COE_RETRY		0
#define	ECMAILBOX_DEFAULT_COE_TMOUT		1000	// timeout for a sdo response from the server
#define	ECMAILBOX_DEFAULT_FOE_RETRY		0
#define	ECMAILBOX_DEFAULT_FOE_BUSY		250		// timeout for a foe response from the server
#define	ECMAILBOX_DEFAULT_FOE_TMOUT		1000	// timeout for a foe response from the server
#define	ECMAILBOX_DEFAULT_SOE_RETRY		3
#define	ECMAILBOX_DEFAULT_SOE_TMOUT		1000	


#define	ECSLAVE_MSGTYPE	ADSLOG_MSGTYPE_LERROR

#define	MIN_ECSLAVE_DELAY	100 // ns


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// EcSyncClient


#define DEFAULT_SYNCTIMEOUT 5000;

typedef struct TEcSyncDevReq
{
	void *						hEvent;	
	unsigned long				nInvokeId;			
	unsigned long				nResult;			
	unsigned long				cbLength;
	PETYPE_EC_HEADER	pFrame;
}EcSyncDevReq, *PEcSyncDevReq;

typedef struct TEcSyncMbReq
{
	void *						hEvent;		
	unsigned long				nInvokeId;
	unsigned long				cbLength;
	unsigned long				nResult;
	unsigned short				bReceived;
	unsigned char *				pData;	
}EcSyncMbReq, *PEcSyncMbReq;




#pragma pack( )
