#ifndef THREADSBUILDER_H
#define THREADSBUILDER_H

#include <QApplication>
#include <unistd.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <limits.h>
#include <iostream>

#include "easycatinterfaceclass.h"

#define MY_STACK_SIZE (10*1024*1024)      // 10 Mb
#define nSecondsInMillis 1000000*500
#define nSecondsInSeconds 1000000000

struct period_info {
    struct timespec next_period;
    long period_ns;
};

using namespace std;


static void ConfigureMemoryLocks()
{
    if (mlockall(MCL_CURRENT | MCL_FUTURE)) perror("mlockall failed:"); //Memory Lock, no page faults
    mallopt(M_TRIM_THRESHOLD, -1); //Turn off memory trimming
    mallopt(M_MMAP_MAX, 0); //Turn off shared memory usage
}

static void LockProcessMemory(int size)
{
    int i;
    char *buffer;
    buffer = (char*)malloc(size);
    for (i = 0; i < size; i += sysconf(_SC_PAGESIZE)) buffer[i] = 0; // Send this memory to RAM and lock it there
    free(buffer);
}

static void PeriodIncrement(struct period_info *periodInfo)
{
    periodInfo->next_period.tv_nsec += periodInfo->period_ns; //Incrementing the NextPeriod interval for sleeping, the timer source is monotonic
    while (periodInfo->next_period.tv_nsec >= nSecondsInSeconds) { //tv_nsec has to be less than nSecondsInSEconds, because seconds are dealt with in tv_sec
        periodInfo->next_period.tv_sec++;
        periodInfo->next_period.tv_nsec -= nSecondsInSeconds;
    }
}

static void WaitUntilPeriodElapsed(struct period_info *periodInfo)
{
    PeriodIncrement(periodInfo); // periodInfo structure handling
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &periodInfo->next_period, NULL); //Just Sleep until the end of the time period required
}

static void *TheRtThread(void *args)
{
    struct period_info periodInfo;
    struct sched_param shedulerParameters;
    shedulerParameters.sched_priority = 98;
    sched_setscheduler(0, SCHED_FIFO, &shedulerParameters);
    periodInfo.period_ns = nSecondsInMillis;
    clock_gettime(CLOCK_MONOTONIC, &(periodInfo.next_period));

    while (1) {
        cout << "Ehy!" << endl;
        WaitUntilPeriodElapsed(&periodInfo);
    }
}


static void StartMasterRtThread()
{
    pthread_t threadRt;
    pthread_attr_t threadAttributes;
    uint8_t error = 0;


    /* init to default values */
    if (pthread_attr_init(&threadAttributes)) {
        cout << "pthread_attr_init failed" << endl;
        error = 1;
    }
    /* Set the requested stacksize for this thread */
    if (pthread_attr_setstacksize(&threadAttributes, PTHREAD_STACK_MIN + MY_STACK_SIZE)) {
        cout << "pthread_attr_setstacksize" << endl;
        error = 1;
    }

    if (!error) pthread_create(&threadRt, &threadAttributes, TheRtThread, NULL);

}

#endif // THREADSBUILDER_H
