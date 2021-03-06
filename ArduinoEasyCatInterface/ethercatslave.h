#ifndef ETHERCATSLAVE_H
#define ETHERCATSLAVE_H

/* Ethercat Slave interface. This class is used as base class
   for any slave. All the effort can be put in the design of our specific
   slave, with in mind that the ethercat slave interface requires
   to overload some function.
   This functions are the ones marked as virtual
*/

#include "ecrt.h"

class EthercatSlave
{
public:
    EthercatSlave() {}
    uint8_t numberOfDomainEntries; // Ethercat utility
    uint16_t alias; // Ethercat utility
    uint16_t position; // Ethercat utility
    uint32_t vendor_id; // Ethercat utility
    uint32_t product_code; // Ethercat utility
    uint8_t id; // Ethercat utility

    ec_pdo_entry_reg_t *domainRegistersPointer; // Ethercat utility
    ec_pdo_entry_info_t *slavePdoEntriesPointer; // Ethercat utility
    ec_pdo_info_t *slavePdosPointer; // Ethercat utility
    ec_sync_info_t *slaveSyncsPointer; // Ethercat utility

    uint8_t *domainDataPointer; // Ethercat utility

    // Function to overload in the slave, it gets called before the cyclical task begins
    virtual void Init(uint8_t *domainDataPointerExt) {domainDataPointer = domainDataPointerExt;}
    virtual void LoopFunction() {} // Function to overload in the slave, it is the slave main function
    virtual void ReadInputs() {} // Function to overload in the slave, you have to specify what to read
    virtual void WriteOutputs() {} // Function to overload in the slave, you have to specify what to write

};

#endif // ETHERCATSLAVE_H
