/////////////////////////////////////////////////////////////////////////////////////////


#define	NUMERO_OGGETTI_MAPPA		128

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void ApriEtherCAT( void );

unsigned char ComandiEtherCAT(  unsigned char * pComando, unsigned char * pDati);

unsigned char TrasferisciConfigurazioneEtherCAT( unsigned char * pDati, unsigned long dwInputBufferSize  );

unsigned char CaricaLeMappeEtherCAT( unsigned char * pDati );

unsigned char InizializzazioneEtherCATSchedaRTL8139(  void );

void GestioneRicezioneSchedaRTL8139(  );

void GestioneTrasmissioneSchedaRTL8139( );

void ChiudiEtherCAT( void );

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////



