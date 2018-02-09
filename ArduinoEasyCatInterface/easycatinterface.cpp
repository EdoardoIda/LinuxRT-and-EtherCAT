#include "easycatinterface.h"
#include "ui_easycatinterface.h"

EasyCatInterface::EasyCatInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EasyCatInterface)
{
    ui->setupUi(this);
    // We have to connect every signal and every slot of our project
    // master+Gui, since they will communicate through them
    connect(&easyCatMaster,&EasyCatMaster::SendStartUp,this,&EasyCatInterface::StartUpReceived);
    connect(&easyCatMaster,&EasyCatMaster::HandShakeUi,this,&EasyCatInterface::HandShakeReceived);
    connect(&easyCatMaster,&EasyCatMaster::SendCallsNumber,this,&EasyCatInterface::ReceiveCallsNumber);
    connect(&easyCatMaster,&EasyCatMaster::SendCyclesNumber,this,&EasyCatInterface::ReceiveCyclesNumber);
    connect(&easyCatMaster,&EasyCatMaster::UpdateBarValue,this,&EasyCatInterface::SetBarValue);
    connect(this,&EasyCatInterface::RequestUpdate,&easyCatMaster,&EasyCatMaster::SetEasyCatUpdate);
    connect(this,&EasyCatInterface::RequestFrequencySetpoint,&easyCatMaster,&EasyCatMaster::SetEasyCatFrequency);

}

EasyCatInterface::~EasyCatInterface()
{
    delete ui;
}

void EasyCatInterface::on_UpdateButton_clicked()
{
    emit RequestUpdate(slave0);
}

void EasyCatInterface::on_UpdateButton2_clicked()
{
    emit RequestUpdate(slave1);
}

void EasyCatInterface::on_frequencySpinBox_editingFinished()
{
    emit RequestFrequencySetpoint(slave0,ui->frequencySpinBox->value());
}

void EasyCatInterface::on_frequencySpinBox2_editingFinished()
{
    emit RequestFrequencySetpoint(slave1,ui->frequencySpinBox2->value());
}

void EasyCatInterface::StartUpReceived()
{
    ui->SlaveBrowser->append("Master Has Started");
}

void EasyCatInterface::SetBarValue(int id)
{
    switch (id) {
        case slave0: {
            ui->progressBar->setValue(ui->frequencySpinBox->value());
            break;
        }
        case slave1: {
            ui->progressBar2->setValue(ui->frequencySpinBox2->value());
            break;
        }
    }
 }

void EasyCatInterface::HandShakeReceived()
{
     ui->SlaveBrowser->append("Master is processing the updates");
}

void EasyCatInterface::ReceiveCallsNumber(int id, int numberOfCalls)
{
    QString str1;
    QString str2;
    ui->SlaveBrowser->append("Led brightness on Slave " + str1.setNum(id) + " has changed "+ str2.setNum(numberOfCalls) + " times.");
}

void EasyCatInterface::ReceiveCyclesNumber(int id, int numberOfCycles)
{
    QString str1;
    QString str2;
    ui->SlaveBrowser->append("Slave " + str1.setNum(id) + " cycles intercurring between every master call = " + str2.setNum(numberOfCycles));
}
