#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/time.h>
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

std::string& replace_all_distinct(std::string& str,const std::string& old_value,const std::string& new_value)     
{     
    for(std::string::size_type pos(0); pos!=std::string::npos; pos+=new_value.length())
    {     
        if( (pos = str.find(old_value,pos)) != std::string::npos)     
            str.replace(pos,old_value.length(),new_value);     
        else break;     
    }     
    return str;     
} 

std::string legalization_file_name(std::string s)
{
    string r = replace_all_distinct(s,"/","-");
    r = replace_all_distinct(r,"!","-");
    r = replace_all_distinct(r,"*","-");
    r = replace_all_distinct(r,"\\","-");
    r = replace_all_distinct(r,"?","-");
    r = replace_all_distinct(r,"<","-");
    r = replace_all_distinct(r,">","-");
    r = replace_all_distinct(r,"\"","-");
    return s;
}

int main(int argc, char *argv[])
{
    struct timeval tpstart,tpend;
    double timeuse;
    

    //string s = "313923_ߒԦ⡦ď思ߒԀrefs/com.c!";
    string s = "313923_ߒԦ⡦ď!思ߒԀrefs/com/c!?*<>\\\"";
    gettimeofday(&tpstart,NULL);
    string r;
    r = legalization_file_name(s);
    cout << r << endl;
    gettimeofday(&tpend,NULL);
    
        
    timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
    timeuse/=1000000;
    cout << timeuse << "s" << endl;
}