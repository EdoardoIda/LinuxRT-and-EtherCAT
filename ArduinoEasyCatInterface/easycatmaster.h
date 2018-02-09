#ifndef EASYCATMASTER_H
#define EASYCATMASTER_H

/* EasyCAT Master class. Inherits Ethercat Master base class
 * It is fundamental to inherit BOTH Qobject (signal and slot mechanism,
 * first base class) and Ethercat master
*/

#include <QObject>

#include "ethercatmaster.h"
#include "easycatslave.h"

class EasyCatMaster : public QObject, public EthercatMaster
{
    Q_OBJECT
private:
    constexpr static uint8_t slavesNumber = 2; // Some easycat master specific infos
    constexpr static uint8_t EasyCatNumber = 2;
    constexpr static uint8_t numberOfMasterStates = 2;
    constexpr static uint8_t update = 1;
    constexpr static uint8_t notUpdate = 0;

    typedef void (EasyCatMaster::*StateFunction)(); // Easy way to implement state machine
    enum MasterState {
        idle,
        updateSlaves,
    } state,masterFlags; // state machine utilities
    // State machine function array
    StateFunction stateMachine[numberOfMasterStates] = {&EasyCatMaster::IdleFun,
                                                        &EasyCatMaster::UpdateSlavesFun};
    // State machine transition function array

    StateFunction stateManager[numberOfMasterStates] = {&EasyCatMaster::IdleTransition,
                                                        &EasyCatMaster::UpdateSlavesTransition};
    uint16_t *currentSlave = NULL;

    void IdleFun(); // State functions
    void UpdateSlavesFun();
    void IdleTransition(); // State transition functions
    void UpdateSlavesTransition();

    void UpdateUi();

public:
    explicit EasyCatMaster(QObject *parent = nullptr);
     ~EasyCatMaster();
    /* The next 2 array are fundamental. The first one contains all the slaves
     * of a specified type (easycat, in this case), and for each kind of ethercat slave
     * we have to define a separate array.
     * The *etherCatSlave pointer array contains the addresses of all the slave on the
     * ethercat bus, in the order they are mapped in the bus. So the master base class can deal with
     * everything ethercat related, without the user having to worry about it
    */
    EasyCatSlave easyCatSlave[EasyCatNumber] = {{EasyCatSlave(0)},
                                                {EasyCatSlave(1)}};
    EthercatSlave *etherCatSlave[slavesNumber] = {{&easyCatSlave[0]},
                                                  {&easyCatSlave[1]}};

    virtual void StartUpFunction(); //overloaded master ethercat functions, we have to overload them
    virtual void LoopFunction();

// Signals and slots to communicate with the Ui
signals:
    void SendStartUp();
    void HandShakeUi();
    void SendCallsNumber(int,int);
    void SendCyclesNumber(int,int);
    void UpdateBarValue(int);
public slots:
    void SetEasyCatUpdate(int id);
    void SetEasyCatFrequency(int id, int frequency);
};

#endif // EASYCATMASTER_H
