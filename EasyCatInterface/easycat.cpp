#include "easycat.h"

EasyCAT::EasyCAT(QObject *parent) : QObject(parent)
{
    this->slaveStatus = 1;
    outputPdos.slaveStatus = this->slaveStatus;
}

EasyCAT::~EasyCAT() {}


void EasyCAT::SetUpdateFlag()
{
    this->slaveUpdate = 1;

}

void EasyCAT::SetFrequency(const QString &frequencyString)
{
    this->ledFrequency = frequencyString.toUShort();
}

void EasyCAT::Task(uint8_t *domainDataPointer, OffsetIn *offsetIn, OffsetOut *offsetOut)
{
    inputPdos.handShaker = EC_READ_U8(domainDataPointer + offsetIn->handShaker);    // First we process the inputs
    inputPdos.numberOfCalls = EC_READ_U8(domainDataPointer + offsetIn->numberOfCalls);
    inputPdos.cycleCounter = EC_READ_U8(domainDataPointer + offsetIn->cycleCounter);

    // Then we start processing the outputs whitin the application
    if (this->slaveUpdate) {  //If the update flag is set, tell the slave
        outputPdos.ledFrequency = this->ledFrequency;
        outputPdos.slaveUpdate = this->slaveUpdate;
        this->slaveUpdate = 0;
    } else if (inputPdos.handShaker && outputPdos.slaveUpdate) { //if we are still waiting for an answer and the answer arrives, reset the update flag
        outputPdos.slaveUpdate = slaveUpdate;
        this->handShaker = inputPdos.handShaker;
        this->numberOfCalls = inputPdos.numberOfCalls;
        emit SendUpdateReceived(inputPdos.numberOfCalls);
    } else if (handShaker) { // if we have reset the update flag, this time the slave is communicating the number of cycles between two master call, let's get them
        this->handShaker = 0;
        this->cycleCounter = inputPdos.cycleCounter;
        emit SendCycleNumber(inputPdos.cycleCounter);
    }

    EC_WRITE_U8(domainDataPointer + offsetOut->slaveStatus, outputPdos.slaveStatus);
    EC_WRITE_U8(domainDataPointer + offsetOut->slaveUpdate, outputPdos.slaveUpdate);
    EC_WRITE_U8(domainDataPointer + offsetOut->ledFrequency, outputPdos.ledFrequency);
}
