#include <stdio.h>
#include <fcgi_stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <iostream>
using namespace std;

int url_decode(char *str, int len);
void sigterm_handler(int arg);
volatile int _running = 1;

void init_daemon();
bool IsDirectoryExist(const char* path);
typedef std::map<std::string,std::string> _MAP_PARAMS;
_MAP_PARAMS GetRequestParams(const FCGX_Request* pRequest);
int FileWriteString(string s,FILE* fp);
void show_help();
void log(const char* sz);
    
int main(int argc, char *argv[])
{
    printf("dd\n");
    bool bDeamon = false;
    int operation;
    //通过while循环获取  
    while((operation = getopt(argc, argv, "dh")) != -1)  
    {  
        switch(operation)  
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

    const char* logpath = "/data/www/report/crash";
    if(!IsDirectoryExist(logpath))
    {
        if(mkdir(logpath,S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO) == -1)
        {
            cerr << "Create directory " << logpath << " error!" << endl;
            cerr << strerror(errno) << endl;
            return 0;
        }
    }

    if(bDeamon)
    {
        init_daemon();
        signal(SIGTERM, sigterm_handler);
        signal(SIGCHLD, SIG_IGN);
    }

     log("crashreport run.");

    FCGX_Init();
    FCGX_Request fcgiRequest;
    int sockfd = FCGX_OpenSocket("127.0.0.1:6000",100);
    FCGX_InitRequest(&fcgiRequest,sockfd,0);
    while (FCGX_Accept_r(&fcgiRequest) >= 0)
    {
        FCGX_FPrintF(fcgiRequest.out,"Content-type: text/html\r\n\r\n");
        char* method = FCGX_GetParam("REQUEST_METHOD",fcgiRequest.envp);
        if (0 == strcmp(method, "POST"))
        {
            _MAP_PARAMS params = GetRequestParams(&fcgiRequest);
            
            time_t timer;
            struct tm* t_tm;
            time(&timer);
            t_tm = localtime(&timer);
            
            char path[256] = {0}; 

            //按日期
            sprintf(path,"%s/%02d-%02d/",logpath,t_tm->tm_mon+1,t_tm->tm_mday);
            if(!IsDirectoryExist(path))
            {
                mkdir(path,S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
            }

            if (params.find("project") == params.end())
            {
                FCGX_FPrintF(fcgiRequest.out,"bug record fail!missing project name.\n");
                continue;
            }

            //按项目名
            string project = params["project"];
            if (project.length() == 0)
            {
                FCGX_FPrintF(fcgiRequest.out,"bug record fail!missing project name.\n");
                continue;
            }

            strcat(path,project.c_str());
            strcat(path,"/");

            if(!IsDirectoryExist(path))
            {
                mkdir(path,S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
            }

            string userinfo = "";
            if (params.find("userid") != params.end())
            {
                userinfo = "-" + params["userid"];
            }
            if (params.find("username") != params.end())
            {
                userinfo += "-" + params["username"];
            }

            //按时间
            char filename[128] = {0};
            sprintf(filename,"%02d%02d%02d%s.log",t_tm->tm_hour,t_tm->tm_min,t_tm->tm_sec,userinfo.c_str());
            strcat(path,filename);

            FILE* fp = NULL;
            if((fp = fopen(path,"w")) == NULL)
            {
                std::stringstream ss;
                ss << "open logfile fail err = " << errno << " file: " << path ;
                log(ss.str().c_str());
                FCGX_FPrintF(fcgiRequest.out,"crash record fail!open logfile fail.\n");
                continue;
            }

            _MAP_PARAMS::iterator it = params.begin();
            while(it != params.end())
            {
                if (it->first != "content")
                {
                    FileWriteString(it->first,fp);
                    FileWriteString(" = ",fp);
                    FileWriteString(it->second,fp);
                    FileWriteString("\n",fp);
                }
                it++;
            }

            it = params.find("content");
            if (it != params.end())
            {
                FileWriteString("\n-------------------------------------------------------------------------------------------------------------------\n",fp);
                FileWriteString(it->second,fp);
                FileWriteString("\n\r",fp);
            }
            
            fclose(fp);
            
            FCGX_FPrintF(fcgiRequest.out,"bug record success!\n");
        }
        else
        {
            FCGX_FPrintF(fcgiRequest.out,"requst mothod only by post!\n");
        }
    }
}

void show_help()
{
    cout << endl;
    cout << "    -d -- deamon     Run in deamon mode." << endl;
    cout << "    -h -- help       Show help." << endl;
    cout << endl;
}

void log(const char* sz)
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

    cout << sztime << sz << endl;
}

void init_daemon()
{
    pid_t pid = fork();

    if(pid < 0) //1. fork , father process exit
    {
        perror("error fork");
        exit(1);
    }
    else if(pid == 0)
    {
        ;
    }
    else 
    {
        //退出父进程
        exit(0);
    }

    setsid();

    chdir("/");
    umask(0);

    int i = 0;
    for(i = 0; i < getdtablesize(); i++)
        close(i);
}

int FileWriteString(string s,FILE* fp)
{
    if (fp != NULL)
    {
        fwrite((void*)s.c_str(),s.length(),1,fp);
    }
    return 0;
}

bool IsDirectoryExist(const char* path)
{
    struct stat fs;
    if (stat(path, &fs) == -1)
    {
        return false;
    }

    if ((fs.st_mode & S_IFMT) == S_IFDIR)
    {
        return true;
    }

    return false;
}

_MAP_PARAMS GetRequestParams(const FCGX_Request* pRequest)
{
    int len = atoi(FCGX_GetParam("CONTENT_LENGTH",pRequest->envp));
    char *bufp = (char*)malloc(len + 1);
    memset(bufp,0,len + 1);
    FCGX_GetStr(bufp,len,pRequest->in);

    vector<string> vt;

    char* _token = strtok(bufp,"&");
    while(_token != NULL)
    {
        vt.push_back(_token);
        _token = strtok(NULL,"&");
    }

    delete[] bufp;
    bufp = NULL;

    _MAP_PARAMS mapParams;
    mapParams.clear();

    unsigned int i = 0;
    for (;i < vt.size(); ++i)
    {
        bufp = new char[vt[i].length() + 1];
        memset(bufp,0,vt[i].length() + 1);
        strcpy(bufp,vt[i].c_str());

        char* key = strtok(bufp,"=");
        char* value= strtok(NULL,"=");

        if (key != NULL && value != NULL)
        {
            url_decode(value,strlen(value));
            mapParams.insert(std::pair<string,string>(key,value));
        }

        delete[] bufp;
        bufp = NULL;

    }
    
    return mapParams;
}


void sigterm_handler(int arg)
{
    _running = 0;
} 


static int php_htoi(char *s)
{
    int value;
    int c;

    c = ((unsigned char *)s)[0];
    if (isupper(c))
        c = tolower(c);
    value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

    c = ((unsigned char *)s)[1];
    if (isupper(c))
        c = tolower(c);
    value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

    return (value);
}

int url_decode(char *str, int len)
{
    char *dest = str;
    char *data = str;

    while (len--) 
    {
        if (*data == '+') 
        {
            *dest = ' ';
        }
        else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2))) 
        {
            *dest = (char) php_htoi(data + 1);
            data += 2;
            len -= 2;
        } 
        else 
        {
            *dest = *data;
        }
        data++;
        dest++;
    }
    *dest = 0;
    return dest - str;
}
