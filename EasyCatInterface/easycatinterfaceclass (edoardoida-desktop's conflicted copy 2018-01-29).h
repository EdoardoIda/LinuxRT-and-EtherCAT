#ifndef EASYCATINTERFACECLASS_H
#define EASYCATINTERFACECLASS_H

#include <QMainWindow>

namespace Ui {
class EasyCatInterfaceClass;
}

class EasyCatInterfaceClass : public QMainWindow
{
    Q_OBJECT

public:
    explicit EasyCatInterfaceClass(QWidget *parent = 0);
    ~EasyCatInterfaceClass();

private slots:
    void on_UpdateButton_clicked();

    void on_spinBox_valueChanged(int arg1);

private:
    Ui::EasyCatInterfaceClass *ui;
};

#endif // EASYCATINTERFACECLASS_H
