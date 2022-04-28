#ifndef _TIMER_MANAGER_H_
#define _TIMER_MANAGER_H_
#include <fstream>
#include <pthread.h>
#include <sys/time.h>
#include <map>

typedef int (*LP_TIMER_PROCFUNC)(int, void *);

struct TTimer
{
    LP_TIMER_PROCFUNC timer_func;
    time_t interval;
    bool loop;
    void *data;
};

typedef std::map<int, TTimer> timers_map;

class CTimerManager
{
public:
    CTimerManager();
    ~CTimerManager();

public:
    int NewTimer();
    int SetTimer(int timerfd, time_t interval, LP_TIMER_PROCFUNC timer_func, bool loop = true, void *data = NULL);

private:
    int epfd;
    pthread_t tid;
    timers_map timers;
};

#endif