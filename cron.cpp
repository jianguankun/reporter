#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdint.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <libgen.h>
#include <dirent.h>

#include <iostream>
using namespace std;

#include "logger.h"
#include "timer_manager.h"

void sigterm_handler(int arg);
volatile int _running = 1;
CLogger *_log = NULL;

void init_daemon();
bool IsDirectoryExist(const char *path);
void show_help();
int timer_proc1(int, void *);
int timer_proc2(int, void *);
bool is_prog_run();
time_t get_next_halfhour();
time_t get_next_day();
void log_next(time_t next,const char *tag);
string shell_exec(const char *command);

int main(int argc, char *argv[])
{
    bool bDeamon = false;
    int operation;
    //通过while循环获取
    while ((operation = getopt(argc, argv, "dh")) != -1)
    {
        switch (operation)
        {
        case 'd':
        {
            bDeamon = true;
            break;
        }
        case 'h':
        {
            show_help();
            return 0;
        }
        }
    }

    if (bDeamon)
    {
        if (is_prog_run())
        {
            cout << "ERROR:one aplication is running" << endl;
            exit(EXIT_FAILURE);
        }
        init_daemon();
        signal(SIGTERM, sigterm_handler);
        signal(SIGCHLD, SIG_IGN);
    }

    CLogger log("/var/reporter/logs/out.log");
    _log = &log;
    log.out() << endl;
    log.out() << endl;
    log.out() << "----------------------------------------------------" << endl;
    log.out() << "CRON started." << endl;
    log.out() << "----------------------------------------------------" << endl;
    CTimerManager ctm;
    int timerfd = -1;
    time_t next = 0;

    timerfd = ctm.NewTimer();
    next = get_next_halfhour();
    log_next(next,"daliy");
    ctm.SetTimer(timerfd, next, timer_proc1, false, (void *)&ctm);

    timerfd = ctm.NewTimer();
    next = get_next_day();
    log_next(next,"half-hour");
    ctm.SetTimer(timerfd, next, timer_proc2, false, (void *)&ctm);

    log.out() << endl;

    while (true)
    {
        sleep(1);
    }

    exit(EXIT_SUCCESS);
}

void show_help()
{
    cout << endl;
    cout << "    -d -- deamon     Run in deamon mode." << endl;
    cout << "    -h -- help       Show help." << endl;
    cout << endl;
}

void init_daemon()
{
    pid_t pid = fork();

    if (pid < 0) // 1. fork , father process exit
    {
        perror("error fork");
        exit(1);
    }
    else if (pid == 0)
    {
        ;
    }
    else
    {
        exit(0);
    }

    setsid();

    chdir("/");
    umask(0);

    int i = 0;
    for (i = 0; i < getdtablesize(); i++)
        close(i);
}

void sigterm_handler(int arg)
{
    _running = 0;
}

int timer_proc1(int timerfd, void *data)
{
    _log->out() << "Half-hour task." << endl;

    const char* command = "sh -c /usr/local/report/hourly/create-crash-analyse.sh 2>&1";
    _log->out(false) << command << endl;
    string ret = shell_exec(command);
    _log->out(false) << ret << endl;

    time_t next = get_next_halfhour();
    log_next(next,"Half-hour");
    CTimerManager *_ctm = (CTimerManager *)data;
    if (NULL != _ctm)
    {
        _ctm->SetTimer(timerfd, next, timer_proc1, false, _ctm);
    }
    return 0;
}

int timer_proc2(int timerfd, void *data)
{
    _log->out() << "Daliy task. ************************************" << endl;

    const char* command = "sh -c /usr/local/report/daliy/create-crash-analyse.sh 2>&1";
    _log->out(false) << command << endl;
    string ret = shell_exec(command);
    _log->out(false) << ret << endl;

    time_t next = get_next_day();
    log_next(next,"daliy");
    CTimerManager *_ctm = (CTimerManager *)data;
    if (NULL != _ctm)
    {
        _ctm->SetTimer(timerfd, next, timer_proc2, false, _ctm);
    }

    return 0;
}

bool is_prog_run()
{
    long pid;
    char full_name[1024] = {0};
    char proc_name[1024] = {0};
    int fd;

    pid = getpid();
    // cout<<"pid:"<<pid<<endl;
    sprintf(full_name, "/proc/%ld/cmdline", pid);
    if (access(full_name, F_OK) == 0)
    {
        fd = open(full_name, O_RDONLY);
        if (fd == -1)
            return false;
        // lseek(fd, 184, SEEK_SET);
        read(fd, proc_name, 1024);
        close(fd);
    }
    else
    {
        return false;
    }
    // cout<<"proc_name:"<<proc_name<<endl;
    char self_proc_name[512] = {0};
    char *p = proc_name;
    int pt = 0;
    while (*p != ' ' && *p != '\0')
    {
        self_proc_name[pt] = *p;
        p++;
        pt++;
    }
    // cout<<"self_proc_name:"<<self_proc_name<<endl;
    string self_final_name = basename(self_proc_name);
    // cout<<"self_final_name:"<<self_final_name<<endl;
    DIR *dir;
    struct dirent *result;
    dir = opendir("/proc");
    while ((result = readdir(dir)) != NULL)
    {
        if (!strcmp(result->d_name, ".") || !strcmp(result->d_name, "..") || !strcmp(result->d_name, "self") || !strcmp(result->d_name, "thread-self") || atol(result->d_name) == pid)
            continue;

        memset(full_name, 0, sizeof(full_name));
        memset(proc_name, 0, sizeof(proc_name));
        sprintf(full_name, "/proc/%s/cmdline", result->d_name);
        if (access(full_name, F_OK) == 0)
        {
            fd = open(full_name, O_RDONLY);
            if (fd == -1)
                continue;
            // lseek(fd, 184, SEEK_SET);
            read(fd, proc_name, 1024);
            close(fd);

            char *q = proc_name;
            pt = 0;
            memset(self_proc_name, 0, sizeof(self_proc_name));
            while (*q != ' ' && *q != '\0')
            {
                self_proc_name[pt] = *q;
                q++;
                pt++;
            }
            string other_final_name = basename(self_proc_name);
            if (self_final_name == other_final_name)
            {
                // cout<<"other_final_name:"<<other_final_name << " " << full_name <<endl;
                return true;
            }
        }
    }
    return false;
}

time_t get_next_halfhour()
{
    time_t t1;
    time_t t2;
    struct tm *tm1;
    struct tm *tm2;

    time(&t1);
    tm1 = localtime(&t1);
    // printf("T1 %ld: %d-%d-%d %d:%d:%d\n", t1,
    //        tm1->tm_year + 1900, tm1->tm_mon + 1, tm1->tm_mday,
    //        tm1->tm_hour, tm1->tm_min, tm1->tm_sec);

    tm2 = tm1;
    tm2->tm_min = 30;
    tm2->tm_sec = 0;
    t2 = mktime(tm2);
    if (t2 - t1 <= 0)
    {
        tm2->tm_min += 30;
        t2 = mktime(tm2);
    }

    // printf("T2 %ld: %d-%d-%d %d:%d:%d\n", t2,
    //        tm2->tm_year + 1900, tm2->tm_mon + 1, tm2->tm_mday,
    //        tm2->tm_hour, tm2->tm_min, tm2->tm_sec);
    // printf("diff : %ld\n", t2 - t1);

    return t2;
}

time_t get_next_day()
{
    time_t t1;
    time_t t2;
    struct tm *tm1;
    struct tm *tm2;

    time(&t1);
    tm1 = localtime(&t1);
    // priountf("T1 %ld: %d-%d-%d %d:%d:%d\n", t1,
    //        tm1->tm_year + 1900, tm1->tm_mon + 1, tm1->tm_mday,
    //        tm1->tm_hr, tm1->tm_min, tm1->tm_sec);

    tm2 = tm1;
    tm2->tm_mday += 1;
    tm2->tm_hour = 0;
    tm2->tm_min = 0;
    tm2->tm_sec = -1;
    t2 = mktime(tm2);
    if (t2 - t1 <= 0)
    {
        tm2->tm_mday += 1;
        t2 = mktime(tm2);
    }

    // printf("T2 %ld: %d-%d-%d %d:%d:%d\n", t2,
    //        tm2->tm_year + 1900, tm2->tm_mon + 1, tm2->tm_mday,
    //        tm2->tm_hour, tm2->tm_min, tm2->tm_sec);
    // printf("diff : %ld\n", t2 - t1);

    return t2;
}

void log_next(time_t next,const char* tag)
{
    if (NULL == _log)
    {
        return;
    }
    struct tm *tm = localtime(&next);
    char sztime[64] = {0};
    //按日期
    sprintf(sztime, "%04d/%02d/%02d/ %02d:%02d:%02d ",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);
    _log->out() << "next " << tag <<" task run at: " << sztime << endl;
    return;
}

string shell_exec(const char *command)
{
    string out = "";
    FILE *fp;
    fp = popen(command,"r");
    if (fp == NULL)
    {
        out = strerror(errno);
    }
    else
    {
        const int READBLOCK = 128;
        char _temp_buf[READBLOCK] = {0};
        char *readbuf = NULL;
        int totallen = 0,readlen = 0;
        while (!feof(fp))
        {
            
            memset(_temp_buf, 0, READBLOCK);
            if ((readlen = fread(_temp_buf, 1,READBLOCK, fp)) > 0)
            {
                totallen += readlen;
                if (feof(fp)) totallen += 1;
                readbuf = (char*)realloc(readbuf,totallen);
                char *p = readbuf + totallen - readlen;
                if (feof(fp)) p--;
                memset(p, 0, readlen);
                memcpy(p,_temp_buf,readlen);
            }
        }

        if(readbuf != NULL)
        {
            out = readbuf;
            free(readbuf);
            readbuf = NULL;
        }
    
        pclose(fp);

    }

    return out;
}