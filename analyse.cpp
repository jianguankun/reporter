#include <stdio.h>
#include <iomanip>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "sha1.h"

using namespace std;

#define MAX_PATH 512

//获取文件的sha1码
string get_file_sha1(const char* szFile);

//获取buffer的sha1码
string get_buffer_sha1(unsigned int buflen,const char* inputbuf);

//文件的扩展名
string get_file_extname(const char* szFile);

//写文件
int FileWriteString(string s,FILE* fp);


void show_help()
{
    cout << "    -r -- report     Generate day report." << endl;
    cout << "    -h -- help       Show help." << endl;
    cout << endl;
}

bool verbose = false;
bool silent = false;

struct TCrashNode
{
    string report;
    int times;
};

typedef map<string,TCrashNode> AnalyseMap;


int main(int argc, char *argv[])
{
    bool bCreateDayReport = false;
    int operation;
    //通过while循环获取  
    while((operation = getopt(argc, argv, "rh")) != -1)  
    {  
        switch(operation)  
        {
        case 'r': 
            {
                bCreateDayReport = true;
                break;
            }
		case 'h': 
			{
				show_help();
				return 0;
			}
        }  
    }

    char targetpath[MAX_PATH] = {0};
    if (optind == argc)
    {
        getcwd(targetpath, MAX_PATH);
    }
    else 
    {
        strcpy(targetpath,argv[optind]);
    }

    // if(bCreateDayReport){
    //     cout << "bCreateDayReport" << endl;
    // }

    // cout << targetpath << endl;
    // return 0;

    if((access(targetpath,F_OK)) == -1)   
    {   
        cerr << "Directory \"" << targetpath << "\" is not exist!" << endl;
        return 0;
    }

    DIR* dfd;
    char item[MAX_PATH];
    struct dirent* _dp;
    if ((dfd = opendir(targetpath)) == NULL)
    {
        cerr << "can't open " << targetpath << endl;
        cerr << strerror(errno) << endl;
        return 0;
    }

    char szbuf[4096] = {0};
    AnalyseMap Analyse;

    while ((_dp = readdir(dfd)) != NULL)
    {
        if (strncmp(_dp->d_name, ".", 1) == 0)
        {
            //跳过当前目录和上一层目录以及隐藏文件
            continue; 
        }
        if (strlen(targetpath) + strlen(_dp->d_name) + 1 >= MAX_PATH)
        {
            cerr << "name " << targetpath << _dp->d_name << "too long" << endl;
            continue; 
        }

        memset(item,0,sizeof(item));
        sprintf(item,"%s/%s", targetpath,_dp->d_name);

        struct stat fs;
        if (stat(item, &fs) == -1)
        {
            cerr << "cannot access the file : " << item;
            continue; 
        }

        if ((fs.st_mode & S_IFMT) == S_IFDIR)
        {
            ;
        }
        else
        {
            if(get_file_extname(item) != "log")
            {
                continue;
            }

            FILE* fp = fopen(item,"rb");
            if(NULL == fp)
            {
                continue;
            }

            while (!feof(fp)) 
            { 
                ::memset(szbuf,0,sizeof(szbuf));
                fgets(szbuf,sizeof(szbuf),fp);
                if(strncmp(szbuf,"--------",8) == 0)
                {
                    ::memset(szbuf,0,sizeof(szbuf));
                    fread(szbuf,sizeof(char),sizeof(szbuf),fp);
                    break;
                }
            }

            string sSHA1 = get_buffer_sha1(strlen(szbuf),szbuf);

            AnalyseMap::iterator it = Analyse.find(sSHA1);
            if(Analyse.find(sSHA1) == Analyse.end())
            {
                TCrashNode node;
                node.report = szbuf;
                node.times = 1;
                Analyse.insert(pair<string,TCrashNode>(sSHA1,node));
            }
            else
            {
                it->second.times++;
            }
            
            fclose(fp);
        }
    }

    closedir(dfd);

    if(Analyse.size() == 0)
    {
        cerr << "error,no log file." << endl;
        return 0;
    }

    char szAnalyseFile[MAX_PATH] = {0};
    char writebuf[512] = {0};
    sprintf(szAnalyseFile,"%s/当日汇总.txt",targetpath);
    FILE* fp = NULL;
    if((fp = fopen(szAnalyseFile,"w")) == NULL)
    {
        return 0;
    }

    const char* prefix = "/data/www/report/";
    if(strncmp(targetpath,prefix,strlen(prefix)) == 0)
    {
        FileWriteString(&targetpath[strlen(prefix)],fp);
        FileWriteString("\n\n\n",fp);
    }

    int iCount = 0;
    int affected = 0;
    AnalyseMap::iterator it = Analyse.begin();
    while(it != Analyse.end())
    {
        iCount++;
        FileWriteString("\n====================================================================================================\n",fp);
        sprintf(writebuf,"编号:%d\n",iCount);
        FileWriteString(writebuf,fp);
        sprintf(writebuf,"出现次数:%d\n",it->second.times);
        FileWriteString(writebuf,fp);
        FileWriteString("信息: \n",fp);
        FileWriteString(it->second.report,fp);
        FileWriteString("\n",fp);
        affected += it->second.times;
        it++;
    }

    fclose(fp);

    if(bCreateDayReport)
    {
        const char* prefix = "/data/www/report/crash/";
        if(strncmp(targetpath,prefix,strlen(prefix)) == 0)
        {
            char* p = &targetpath[strlen(prefix)];
            std::string sDay = "";
            std::string sProject = "";
            while(*p != 0)
            {
                if(*p == '/')
                {
                    p++;
                    break;
                }
                sDay += *(p++);
            }

            while(*p != 0)
            {
                if(*p == '/')
                {
                    p++;
                    break;
                }
                sProject += *(p++);
            }

            // char szBuf[128];
            
            // sprintf(szBuf,"项目:%s",sProject.c_str());
            // cout << setw(30) << setiosflags(ios::left) << szBuf ;
            // sprintf(szBuf,"Crash数量:%d",(int)Analyse.size());
            // cout << setw(20) << setiosflags(ios::left) << szBuf;
            // sprintf(szBuf,"影响玩家:%d次",affected);
            // cout << setw(20) << setiosflags(ios::left) << szBuf;
            // cout << endl;
            cout << sProject << "|" << Analyse.size() << "|" << affected << endl;
        }
        // 
        // if(strncmp(targetpath,prefix,strlen(prefix)) == 0)
        // {
        //     // FileWriteString(&targetpath[strlen(prefix)],fp);
        //     // FileWriteString("\n\n\n",fp);
        //     char* p = &targetpath[strlen(prefix)];
        //     std::string sDay = "";
        //     while(*p != 0)
        //     {
        //         if(*p == '/')
        //         {
        //             break;
        //         }
        //         sDay += *(p++);
        //     }

        //     const char* dayreportpath = "/root/";
        //     char dayreportfile[MAX_PATH] = {0};
        //     char writebuf[512] = {0};
        //     sprintf(dayreportfile,"%s%s-日报.txt",dayreportpath,sDay.c_str());
        //     FILE* fp = NULL;
        //     bool bNullFile = true;
        //     if((fp = fopen(dayreportfile,"r")) != NULL)
        //     {
        //         bNullFile = (fgetc(fp) == EOF);
        //         fclose(fp);
        //     }

        //     if((fp = fopen(dayreportfile,"a")) == NULL)
        //     {
        //         return 0;
        //     };

        //     if(bNullFile)
        //     {
        //         sprintf(writebuf,"%s日报",sDay.c_str());
        //         FileWriteString(writebuf,fp);
        //         FileWriteString("\n",fp);
        //     }

        //     fclose(fp);
            
        // }
    }
    else
    {
        cout << "Create crash analyse report OK!";
        if(strncmp(targetpath,prefix,strlen(prefix)) == 0)
        {
            cout << " -> " << &targetpath[strlen(prefix)] << endl;
        }
        else
        {
            cout << endl;
        }
    }

    return 0;
}


string get_file_sha1(const char* szFile)
{

    char sha1buf[64] = {0};

    unsigned int szMDTemp[5] = {0};

    const unsigned int READ_BLOCK_SIZE = 2048;
    unsigned char fileBuf[READ_BLOCK_SIZE];

    FILE* fp = fopen(szFile,"rb");
    if(NULL == fp)
    {
        return "";
    }

    size_t nReadlen = 0;

    SHA1 sha;
    sha.Reset();
    do
    {
        nReadlen = fread(fileBuf,1,READ_BLOCK_SIZE,fp);
        if(nReadlen != 0)
        {
            sha.Input(fileBuf,(unsigned int)nReadlen);
        }
    } 
    while (nReadlen == READ_BLOCK_SIZE);

    sha.Result(szMDTemp);

    char* p = sha1buf;
    for (int i = 0; i < 5; ++i)
    {
        sprintf(p,"%.8x",szMDTemp[i]);
        p += 8;
    }

    fclose(fp);

    return sha1buf;
}

string get_buffer_sha1(unsigned int buflen,const char* inputbuf)
{
    char sha1buf[64] = {0};

    unsigned int szMDTemp[5] = {0};

    SHA1 sha;
    sha.Reset();
    sha.Input(inputbuf,(unsigned int)buflen);
    sha.Result(szMDTemp);

    char* p = sha1buf;
    for (int i = 0; i < 5; ++i)
    {
        sprintf(p,"%.8x",szMDTemp[i]);
        p += 8;
    }

    return sha1buf;
}

string get_file_extname(const char* szFile)
{
    int len = strlen(szFile);
    if(0 == len)
    {
        return "";
    }
    string extname = "";
    int i = len - 1;
    while(i >= 0)
    {
        if('.' == szFile[i])
        {
            if(i < len)
            {
                extname = &szFile[i + 1];
            }
            break;
        }
        i--;
    }

    return extname;
    
}

int FileWriteString(string s,FILE* fp)
{
    if (fp != NULL)
    {
        fwrite((void*)s.c_str(),s.length(),1,fp);
    }
    return 0;
}