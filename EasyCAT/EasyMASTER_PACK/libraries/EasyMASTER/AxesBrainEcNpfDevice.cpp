#include <SPI.h>         // needed for Ethernet library communication with the W5100 (Arduino ver>0018)

#include "AxesBrainEcInterface.h"
#include "Strutture.h"
#include "AxesBrainEcNpfDevice.h"
#include "AxesBrainEcDevice.h"
#include "AxesBrainEcNpfDevice.h"

#include "EthercatInterface.h"

extern PEcDevice pEcDevice;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//
// Defines the state of the LAN media
//
typedef enum _NDIS_MEDIA_STATE {
    NdisMediaStateConnected,
    NdisMediaStateDisconnected
} NDIS_MEDIA_STATE, *PNDIS_MEDIA_STATE;



#define OID_GEN_MEDIA_CONNECT_STATUS			0x00010114

unsigned int OpenEcDevice(PEcDevice pEcDevice);	
unsigned int CloseEcDevice(PEcDevice pEcDevice);	
unsigned short CheckFrameEcDevice( PEcDevice pEcDevice, void * pData, unsigned long nData, int &bPending);



unsigned short minS (unsigned short  a, unsigned short  b );
unsigned short maxS (unsigned short  a, unsigned short  b );


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CEcNpfDevice::CEcNpfDevice(ETHERNET_ADDRESS macAdapter) 
{
	m_lRef					 = 1;
	m_pAdapter				 = NULL;
	m_bStopReceiver			 = 0;
	m_pszAdapter			 = NULL;
}


CEcNpfDevice::~CEcNpfDevice()
{
	safe_delete_a(m_pszAdapter);
}

// IUnknown
///////////////////////////////////////////////////////////////////////////////
unsigned long CEcNpfDevice::AddRef()
{
  m_lRef ++;
  
  return m_lRef;
    //return InterlockedIncrement(&m_lRef);
}

///////////////////////////////////////////////////////////////////////////////
unsigned long CEcNpfDevice::Release()
{
    m_lRef --;
    
    //if (InterlockedDecrement(&m_lRef) == 0)
    if (m_lRef == 0)
    {
        delete this;
        return 0;
    }
    return m_lRef;
}





///////////////////////////////////////////////////////////////////////////////////////
long CEcNpfDevice::CheckRecvFrames()
{	
	void * pData;

	//calls CEcDevice::CheckFrame for all queued packets (see CEcNpfDevice::ReadPackets)
	while( pEcDevice->m_pEcNpfDevice->m_listPacket.Remove(pData) )
	{
		
		int bPending =0;
		struct bpf_hdr* pHdr = (struct bpf_hdr*) pData;

		if( !CheckFrameEcDevice( m_pEcDevice, (unsigned char *)pHdr + sizeof(bpf_hdr) , pHdr->bh_caplen, bPending) || !bPending )
		{
		}
	}
	return ECERR_NOERR;	
}

///////////////////////////////////////////////////////////////////////////////////////
void CEcNpfDevice::ReturnFrame(PETHERNET_FRAME pFrame)
{
	if( pFrame )
	{
		struct bpf_hdr* pHdr = (struct bpf_hdr*)((unsigned char*)pFrame - sizeof(bpf_hdr));
	}
}


////////////////////////////////////////////////////////////////////////////////////
unsigned long CEcNpfDevice::GetMediaConnectStatus()
{
	
	return EcMediaStateUndefined;
}

