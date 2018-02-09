#include <QApplication> // Common Qt inslusion

#include "easycatmaster.h" //User includes, the master we programmed,
#include "easycatinterface.h" // And its inteface

EasyCatMaster easyCatMaster; // Declare master as global variable (usefull for qt signal and slot system)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // Qt default
    EasyCatInterface easycatInterface; //the GUI

    //Assign some master properties, if you have to, otherwise they are defaulted
    easyCatMaster.masterData.guiCpuId = 0;
    easyCatMaster.masterData.rtCpuId = 1;
    easyCatMaster.masterData.guiPriority = 60;
    easyCatMaster.masterData.rtPriority = 98;
    easyCatMaster.masterData.nSecondsCycleTime = 1000000;
    easyCatMaster.Start(); // Start the master

    easycatInterface.show(); // Show Gui

    return a.exec(); // Qt default
}
