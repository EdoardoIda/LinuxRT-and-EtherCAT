#ifndef EASYCAT_H
#define EASYCAT_H

#include <QObject>
#include <iostream>

#include "ecrt.h"

#define EasyCat 0x0000079a, 0x00defede //Vendor ID (always the same) and Product code (changes according to configuration, 32+32 in this case
#define EasyCatSlaveParameters 0, 0 //Alias and position of the slave, the first one usually remains 0, the second changes according to the
                                    //position of the slave in the ethercat chain
#define MasterOperational 8
#define SlaveOperational 8
#define DomainOperational 2

static ec_master_t *masterPointer = NULL;
static ec_master_state_t masterState = {};

static ec_domain_t *domainPointer = NULL;
static ec_domain_state_t domainState = {};

static ec_slave_config_t *configPointer = NULL;
static ec_slave_config_state_t configState = {};

static uint8_t *domainDataPointer = NULL;

static struct OffsetIn {
    unsigned int handShaker;
    unsigned int numberOfCalls;
    unsigned int cycleCounter;
} offsetIn;

static struct OffsetOut {
  unsigned int slaveStatus;
  unsigned int slaveUpdate;
  unsigned int ledFrequency;
} offsetOut;

const static ec_pdo_entry_reg_t domainRegisters[6] = {
    {EasyCatSlaveParameters, EasyCat, 0x0005, 0x01, &offsetOut.slaveStatus,NULL},
    {EasyCatSlaveParameters, EasyCat, 0x0005, 0x02, &offsetOut.slaveUpdate,NULL},
    {EasyCatSlaveParameters, EasyCat, 0x0005, 0x03, &offsetOut.ledFrequency,NULL},
    {EasyCatSlaveParameters, EasyCat, 0x0006, 0x01, &offsetIn.handShaker,NULL},
    {EasyCatSlaveParameters, EasyCat, 0x0006, 0x02, &offsetIn.numberOfCalls,NULL},
    {EasyCatSlaveParameters, EasyCat, 0x0006, 0x03, &offsetIn.cycleCounter,NULL}};

using namespace std;

class EasyCAT : public QObject
{
    Q_OBJECT
private:
    uint8_t slaveStatus;
    uint8_t slaveUpdate;
    uint8_t ledFrequency;
    uint8_t handShaker;
    uint8_t numberOfCalls;
    uint8_t cycleCounter;

public:
    explicit EasyCAT(QObject *parent = nullptr);
    ~EasyCAT();

    void Task(uint8_t *domainDataPointer, OffsetIn* offsetIn, OffsetOut* offsetOut);

    struct InputPdos {
        uint8_t handShaker;
        uint8_t numberOfCalls;
        uint8_t cycleCounter;
    } inputPdos;

    struct OutputPdos {
        uint8_t slaveStatus;
        uint8_t slaveUpdate;
        uint8_t ledFrequency;
    } outputPdos;

signals:

    void SendUpdateReceived(int);
    void SendCycleNumber(int);
    void SendStartUp();

public slots:

    void SetUpdateFlag();
    void SetFrequency(const QString &frequencyString);
};

#endif // EASYCAT_H
