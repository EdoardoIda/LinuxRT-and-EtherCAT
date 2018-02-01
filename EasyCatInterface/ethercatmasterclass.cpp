#include "ethercatmasterclass.h"

EthercatMasterClass::EthercatMasterClass() {}

void EthercatMasterClass::CheckDomainState()
{
    ec_domain_state_t domainStateLocal;

    ecrt_domain_state(domainPointer, &domainStateLocal);

    if (domainStateLocal.working_counter != domainState.working_counter) {
        cout << "Domain: WC " << domainStateLocal.working_counter << endl;
    }
    if (domainStateLocal.wc_state != domainState.wc_state) {
        cout << "Domain: State " << domainStateLocal.wc_state << endl;
        if (domainStateLocal.wc_state == DomainOperational) domainStateFlag = 1;
        else domainStateFlag = 0;
    }

    domainState = domainStateLocal;
}

void EthercatMasterClass::CheckMasterState()
{
    ec_master_state_t masterStateLocal;

    ecrt_master_state(masterPointer, &masterStateLocal);

    if (masterStateLocal.slaves_responding != masterState.slaves_responding) {
        cout << masterStateLocal.slaves_responding << " slave(s) on the bus" << endl;
    }
    if (masterStateLocal.al_states != masterState.al_states) {
        cout << " Master states: " <<  masterStateLocal.al_states << endl;
        if (masterStateLocal.al_states == MasterOperational) masterStateFlag = 1;
        else masterStateFlag = 0;
    }
    if (masterStateLocal.link_up != masterState.link_up) {
        cout << "Master Link is " << (masterStateLocal.link_up ? "up" : "down") << endl;
    }

    masterState = masterStateLocal;
}

void EthercatMasterClass::CheckConfigState()
{
    ec_slave_config_state_t configStateLocal;

    ecrt_slave_config_state(configPointer, &configStateLocal);

    if (configStateLocal.al_state != configState.al_state) {
        cout << "Slaves State " << configStateLocal.al_state << endl;
        if (configStateLocal.al_state == SlaveOperational) slaveStateFlag = 1;
        else slaveStateFlag = 0;
    }
    if (configStateLocal.online != configState.online) {
        cout << "Slaves: " << (configStateLocal.online ? "online" : "offline") << endl;
    }
    if (configStateLocal.operational != configState.operational) {
        cout << "Slaves: " << (configStateLocal.operational ? "" : "Not ") << "operational" << endl;
    }

    configState = configStateLocal;
}

uint8_t EthercatMasterClass::InitProtocol()
{

    if (!(masterPointer = ecrt_request_master(0))) { // Requesting to initialize master 0
        cout << "Error requesting master" << endl;
        return 1;
    }
    if (!(domainPointer= ecrt_master_create_domain(masterPointer))) { // Creating Domain Process associated with master 0
        cout << "Error Creating Domain" << endl;
        return 1;
    }
    if (!(configPointer = ecrt_master_slave_config(masterPointer, EasyCatSlaveParameters, EasyCat))) {
        cout << "Error Configuring Slave Devices" << endl;
        return 1;
    }
    cout << "Configuring PDOs" << endl;
    if (ecrt_slave_config_pdos(configPointer, EC_END, slave_0_syncs)) {
        cout << "Error Configuring PDOs" << endl;
        return 1;
    }
    if (ecrt_domain_reg_pdo_entry_list(domainPointer, domainRegisters)) {
        cout << "Error Registering PDOs' entries" << endl;
        return 1;
    }
    cout << "Activating master" << endl;
    if (ecrt_master_activate(masterPointer)) {
        cout << "Error activating Master" << endl;
        return 1;
    }
    if (!(domainDataPointer = ecrt_domain_data(domainPointer))) {
        cout << "Error Initializing Domain Data" << endl;
        return 1;
    }

    emit easyCAT.SendStartUp();

    return 0;
}

void EthercatMasterClass::LoopFunction()
{
    ecrt_master_receive(masterPointer); // Collect input data
    ecrt_domain_process(domainPointer);
    CheckDomainState(); //Optional but very recommended
    CheckMasterState(); //Optional
    CheckConfigState(); //Optional
    // Do what you have to here...

    if (masterStateFlag && domainStateFlag && slaveStateFlag) {
        pthread_mutex_lock( &rtMutex );
        easyCAT.Task(domainDataPointer,&offsetIn,&offsetOut);
        pthread_mutex_unlock( &rtMutex );
    }
    // and stop here...
    ecrt_domain_queue(domainPointer);
    ecrt_master_send(masterPointer);    // Send output data
}
