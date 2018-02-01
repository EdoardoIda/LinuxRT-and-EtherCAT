#include "easycatinterfaceclass.h"
#include "ui_easycatinterfaceclass.h"

EasyCatInterfaceClass::EasyCatInterfaceClass(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EasyCatInterfaceClass)
{

    ui->setupUi(this);
    connect(ui->UpdateButton,&QPushButton::clicked,&master.easyCAT,&EasyCAT::SetUpdateFlag);
    connect(ui->frequencySpinBox,static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged),&master.easyCAT,&EasyCAT::SetFrequency);
    connect(&master.easyCAT,&EasyCAT::SendUpdateReceived,this,&EasyCatInterfaceClass::ReceiveUpdateReceived);
    connect(&master.easyCAT,&EasyCAT::SendCycleNumber,this,&EasyCatInterfaceClass::ReceiveCycleNumber);
    connect(&master.easyCAT,&EasyCAT::SendStartUp,this,&EasyCatInterfaceClass::ReceiveStartUp);
}

void EasyCatInterfaceClass::ReceiveUpdateReceived(int numberOfCalls)
{
    QString str;
    ui->SlaveBrowser->append("Led brightness has changed "+ str.setNum(numberOfCalls) + " times.");
}

void EasyCatInterfaceClass::ReceiveCycleNumber(int numberOfCycles)
{
    QString str;
    ui->SlaveBrowser->append("Slave cycles intercurring between every master call = " + str.setNum(numberOfCycles));
}

void EasyCatInterfaceClass::ReceiveStartUp()
{
    ui->SlaveBrowser->append("Starting Up...");
}

EasyCatInterfaceClass::~EasyCatInterfaceClass()
{
    delete ui;
}
