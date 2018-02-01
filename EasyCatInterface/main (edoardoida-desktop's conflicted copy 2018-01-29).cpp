#include <QApplication>

#include "threadsbuilder.h"

#define PRE_ALLOCATION_SIZE (100*1024*1024) /* 100MB pagefault free buffer */

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ConfigureMemoryLocks();
    LockProcessMemory(PRE_ALLOCATION_SIZE);
    EasyCatInterfaceClass Interface;
    StartMasterRtThread();
    Interface.show();
    return a.exec();
}
