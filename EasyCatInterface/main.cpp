#include <QApplication>

#include "threadsbuilder.h"
#include "easycatinterfaceclass.h"

#define PRE_ALLOCATION_SIZE (100*1024*1024) /* 100MB pagefault free buffer */

EthercatMasterClass master;

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EasyCatInterfaceClass Interface;
    ConfigureMemoryLocks();
    LockProcessMemory(PRE_ALLOCATION_SIZE);

    StartMasterRtThread();
    Interface.show();
    return a.exec();
}
