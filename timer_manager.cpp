#include "timer_manager.h"

#include <iostream>
using namespace std;
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "logger.h"
#include <errno.h>

void *thread_worker(void *user);
struct TThread
{
    int epfd;
    timers_map *_timers;
};

CTimerManager::CTimerManager()
{
    timers.clear();
    epfd = epoll_create(1);
    TThread *_data = new TThread();
    _data->epfd = epfd;
    _data->_timers = &timers;
    pthread_create(&tid, NULL, &thread_worker, (void *)_data);
}

CTimerManager::~CTimerManager()
{
}

int CTimerManager::NewTimer()
{
    CLogger log("/var/reporter/logs/thread.log");
    int tfd = -1;
    tfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);
    if (tfd == -1)
    {
        return -1;
    }

    TTimer t = {0};
    timers[tfd] = t;

    log.out() << "new timer timerfd : " << tfd << endl;

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = tfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev);

    return tfd;
}

int CTimerManager::SetTimer(int timerfd, time_t interval, LP_TIMER_PROCFUNC timer_func, bool loop /*= true*/, void *data /*= NULL*/)
{
    CLogger log("/var/reporter/logs/thread.log");

    timers_map::iterator _it = timers.find(timerfd);
    if (_it == timers.end())
    {
        return -1;
    }

    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) == -1)
    {
        return -1;
    }

    // 设置第一次超时时间和超时间隔
    struct itimerspec new_value;
    // 第一次超时时间
    new_value.it_value.tv_sec = interval;
    new_value.it_value.tv_nsec = now.tv_nsec;
    // 设置超时间隔
    new_value.it_interval.tv_sec = loop ? interval : 0;
    new_value.it_interval.tv_nsec = 0;

    if (timerfd_settime(_it->first, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
    {
        return -1;
    }

    _it->second.timer_func = timer_func;
    _it->second.interval = interval;
    _it->second.loop = false;
    _it->second.data = data;

    // log.out() << "set timer timerfd : "
    //           << timerfd << "*" << ends
    //           << timer_func << "*" << ends
    //           << interval << endl;

    return _it->first;
}

void *thread_worker(void *user)
{
    CLogger log("/var/reporter/logs/thread.log");

    ssize_t s;
    uint64_t exp = 0;
    const int MAX_ENENTS = 10;
    struct epoll_event events[MAX_ENENTS] = {0};

    TThread *_data = (TThread *)user;
    int epfd = _data->epfd;
    timers_map *_timers = _data->_timers;

    while (true)
    {
        int n = epoll_wait(epfd, events, MAX_ENENTS, -1);
        if (n > 0)
        {
            for (int i = 0; i < n; ++i)
            {
                int tfd = events[i].data.fd;
                s = read(tfd, &exp, sizeof(uint64_t));
                if (s != sizeof(uint64_t))
                {
                    continue;
                }
                timers_map::iterator _it = _timers->find(tfd);
                if (_it == _timers->end())
                {
                    continue;
                }

                LP_TIMER_PROCFUNC _timerproc = _it->second.timer_func;
                if (NULL != _timerproc)
                {
                    _timerproc(tfd,_it->second.data);
                }
            }
        }
    }
}