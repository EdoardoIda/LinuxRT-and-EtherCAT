Hardware requirements:

1) Arduino DUE
2) Ethernet Shield 2

Installation steps:

1) Install the EtherCAT Configurator tool of Beckhoff
2) Copy the two libraries into Arduino libraries folder normally located into Documents\Arduino\libraries
3) Copy the EasyMASTER_CT directory on your harddisk

Software usage:

1) Using the EtherCAT Configurator tool of Beckhoff to extract the XML file containing the EtherCAT line configuration
2) Using the EasyMASTER_CT tool to convert the XML files into the Header file named Configurazione.h into the EasyMASTER library directry normally located into 
Documents\Arduino\libraries\EasyMASTER
3) Create your Sketch and compile all with the Arduino IDE

Remember:
You can find the offset of the various slaves into the tree window in the EasyMASTER_CT tool, but remember that the indicated offset is in bit and the library is 
addressed in byte, so YOU NEED TO DIVIDE THIS NUMBER BY 8.

In the package there are 3 ready to use XML configuration files:
Beckhoff_4I_4O: for Beckhoff coupler (EK1100) and 4 input (EL1004) and 4 output (EL2004) modules
EasyCAT: for our EasyCAT shield
Beckhoff_4I_4O+EasyCAT: for Beckhoff coupler (EK1100) and 4 input (EL1014) and 4 output (EL2004) module and our EasyCAT shield


Library reference:
void InitMaster(int iCampionatura);
	Call this function to initialize the library in the Setup function of the Sketch passing the sampling time in milliseconds

void ExchangeMaster();
	Call this function cyclically every sampling time to exchange the EtherCAT frames

void WriteBuffer(int iStart, int iLen, char * pLocalBuffer);
	Call this function to Write into the output buffer data of EtherCAT frame

void ReadBuffer(int iStart, int iLen, char * pLocalBuffer);
	Call this function to read from the input buffer data of EtherCAT frame
