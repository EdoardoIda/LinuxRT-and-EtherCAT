#if !defined(AFX_NPFECDEVICE__0B7F9B68_E1B5_4A4D_9732_B1467D64183A__INCLUDED_)
#define AFX_NPFECDEVICE__0B7F9B68_E1B5_4A4D_9732_B1467D64183A__INCLUDED_

struct _ADAPTER;

	///\brief Class for communicating with a network adapter using the npf protocol driver. 
	///
	///This class uses the packet.dll to communicate to a network adapter. The packet.dll is a 
	///user level wrapper for the npf protocol driver.	
class CEcNpfDevice 
{
public:
	///\brief Constructor of CEcNpfDevice
   ///
   ///\param macAdapter MAC address of the network adapter to communicate with.                
	CEcNpfDevice(ETHERNET_ADDRESS	macAdapter = NullEthernetAddress);

	
	///\brief Destructor of CEcNpfDevice    
	virtual ~CEcNpfDevice();
	
	// IUnknown
   virtual unsigned long AddRef(void);
   virtual unsigned long Release(void);
	



	///\brief Reads an ethernet packet from the selected network adapter.
   ///
	///\return result of the call.         			
	
	///\brief Send a ethernet packet to the selected network adapter.
   ///
   ///\param pData the data to be written to the network adapter.
   ///\param nData number of bytes to write to the network adapter.
   ///\return result of the call.      

public:	
	
	///\brief Checks if there are any packets available.
   ///		
	///If there are any packets availabe CEcDevice::CheckFrame is called.	
   ///\return result of the call.      
   virtual long CheckRecvFrames();
		
	///\brief Get the state of the network connection.   
	///		
	///\return State of the connection.         			
	virtual unsigned long GetMediaConnectStatus();

	///\brief Return pending frame passed in CheckFrame 
	///
	///\pFrame pending Frame.
	virtual  VOID ReturnFrame(PETHERNET_FRAME pFrame);	
		
private:

public:	
	EcDevice*							m_pEcDevice;	
	ETHERNET_ADDRESS					m_macAdapter;
	EcMaster*							m_pEcMaster;	
	bool								m_bOpened;
	
	bool								m_bStopReceiver;
	long								m_lRef;
	char *								m_pszAdapter;

	struct _ADAPTER*					m_pAdapter;			
	CFiFoList<void *, MAX_NPFPACKETS>	m_listPacket;	
};

#endif //AFX_NPFECDEVICE__0B7F9B68_E1B5_4A4D_9732_B1467D64183A__INCLUDED_
