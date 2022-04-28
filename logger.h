#ifndef _LOGGER_H_
#define _LOGGER_H_
#include <fstream>

class CLogger
{
public:
    CLogger(const char *logfile);
    ~CLogger();
    std::fstream &out(bool date = true);

private:
    std::fstream log;
};

#endif