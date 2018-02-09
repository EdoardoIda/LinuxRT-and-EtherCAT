#include "easycatmaster.h"

void EasyCatMaster::IdleFun()
{

}

void EasyCatMaster::UpdateSlavesFun()
{
    etherCatSlave[*currentSlave]->LoopFunction();
}

void EasyCatMaster::IdleTransition()
{
    if (masterFlags == updateSlaves) {
        masterFlags = idle;
        if (easyCatSlave[*currentSlave].internalState == EasyCatSlave::idle) easyCatSlave[*currentSlave].slaveFlags = EasyCatSlave::updateSlave;
        state = updateSlaves;
        emit HandShakeUi();
    }
}

void EasyCatMaster::UpdateSlavesTransition()
{
    if (easyCatSlave[*currentSlave].internalState == EasyCatSlave::idle) {
        state = idle;
        UpdateUi();
    }
}

void EasyCatMaster::UpdateUi()
{
     emit SendCallsNumber(*currentSlave,(int)easyCatSlave[*currentSlave].inputPdos.numberOfCalls);
     emit SendCyclesNumber(*currentSlave,(int)easyCatSlave[*currentSlave].inputPdos.cycleCounter);
     emit UpdateBarValue((int)*currentSlave);
}

EasyCatMaster::EasyCatMaster(QObject *parent):
    QObject(parent)
{
    /* It is FUNDAMENTAL that the constructor has this form. There are several other ways we can program
     * the slave interface so that we do not have to deal with the assignment of this variables in the constructor,
     * but they will require the user to initialize some of the variables in the main file and then pass the as
     * input variable in the constructor. I find that this way it is easy for "not experienced user" to write all their code
     * in just two files, th .h and .cpp of their actual implementation of the ethercat master
    */

    numberOfSlaves = slavesNumber;
    slave = &etherCatSlave[0];
    for (int i=0; i<numberOfSlaves; i++) domainElementsNumber+= etherCatSlave[i]->numberOfDomainEntries;

    // This is actually application related, can be omitted if not useful
    state = idle;
    masterFlags = idle;
}

EasyCatMaster::~EasyCatMaster()
{

}

void EasyCatMaster::StartUpFunction()
{
    emit SendStartUp();
}

void EasyCatMaster::LoopFunction()
{
    for (int i=0;i<slavesNumber; i++) etherCatSlave[i]->ReadInputs(); // Read all the pdos
    (this->*stateManager[state])(); // check if the state has to be changed
    (this->*stateMachine[state])(); // do your job
    for (int i=0;i<slavesNumber; i++) etherCatSlave[i]->WriteOutputs(); // Write all the pdos
}

void EasyCatMaster::SetEasyCatUpdate(int id)
{
    currentSlave = &easyCatSlave[id].position;
    if (state == idle) {
        masterFlags = updateSlaves;
    }
}

void EasyCatMaster::SetEasyCatFrequency(int id, int frequency)
{
    easyCatSlave[id].outputPdos.ledFrequency = (uint8_t)ceil(((double)frequency)/100.0*255.0);
}
