#ifndef EASYCATINTERFACE_H
#define EASYCATINTERFACE_H

#include <QWidget> // Qt declarations

#include "easycatmaster.h" //User declaration

extern EasyCatMaster easyCatMaster; // Always declare master as extern, we already defined THAT variable

namespace Ui {
class EasyCatInterface;
}

class EasyCatInterface : public QWidget
{
    Q_OBJECT

public:
    explicit EasyCatInterface(QWidget *parent = 0);
    ~EasyCatInterface();
    enum slaves : int {
        slave0 = 0,
        slave1 = 1
    };
private slots:
    void SetBarValue(int id);
    void on_UpdateButton_clicked();
    void on_UpdateButton2_clicked();
    void on_frequencySpinBox_editingFinished();
    void on_frequencySpinBox2_editingFinished();

public slots:
    void StartUpReceived();
    void HandShakeReceived();
    void ReceiveCallsNumber(int id, int numberOfCalls);
    void ReceiveCyclesNumber(int id, int numberOfCycles);
signals:
    void RequestUpdate(int);
    void RequestFrequencySetpoint(int,int);

private:
    Ui::EasyCatInterface *ui;
};

#endif // EASYCATINTERFACE_H
