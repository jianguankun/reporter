#include "logger.h"
#include <iostream>
using namespace std;

CLogger::CLogger(const char* logfile)
{
    log.open(logfile,ios::app|ios::in|ios::out);
}

CLogger::~CLogger()
{
    log.close();
}

std::fstream& CLogger::out(bool date /*= true*/)
{
    if(date)
    {
        time_t timer;
        struct tm* t_tm;
        time(&timer);
        t_tm = localtime(&timer);

        char sztime[64] = {0};

        //按日期
        sprintf(sztime,"[%04d/%02d/%02d/ %02d:%02d:%02d] ",
                    t_tm->tm_year + 1900,t_tm->tm_mon+1,t_tm->tm_mday,
                    t_tm->tm_hour,t_tm->tm_min,t_tm->tm_sec);

        log << sztime << " ";
    }
    
    return log;
}