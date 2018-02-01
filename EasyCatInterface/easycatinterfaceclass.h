#ifndef EASYCATINTERFACECLASS_H
#define EASYCATINTERFACECLASS_H

#include <QMainWindow>

#include "ethercatmasterclass.h"

extern EthercatMasterClass master;

namespace Ui {
class EasyCatInterfaceClass;
}

class EasyCatInterfaceClass : public QMainWindow
{
    Q_OBJECT

public:
    explicit EasyCatInterfaceClass(QWidget *parent = 0);
    ~EasyCatInterfaceClass();

public slots:

    void ReceiveUpdateReceived(int numberOfCalls);
    void ReceiveCycleNumber(int numberOfCycles);
    void ReceiveStartUp();

private:
    Ui::EasyCatInterfaceClass *ui;
};

#endif // EASYCATINTERFACECLASS_H
