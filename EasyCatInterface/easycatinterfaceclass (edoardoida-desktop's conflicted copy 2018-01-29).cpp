#include "easycatinterfaceclass.h"
#include "ui_easycatinterfaceclass.h"

EasyCatInterfaceClass::EasyCatInterfaceClass(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EasyCatInterfaceClass)
{
    ui->setupUi(this);
}

EasyCatInterfaceClass::~EasyCatInterfaceClass()
{
    delete ui;
}

void EasyCatInterfaceClass::on_UpdateButton_clicked()
{

}

void EasyCatInterfaceClass::on_spinBox_valueChanged(int arg1)
{

}
